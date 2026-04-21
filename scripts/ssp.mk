include scripts/env.mk
include scripts/utils.mk

.DEFAULT_GOAL := all

program_name := ssp
program_dir := $(program_name)
out_dir := $(build_dir)/$(program_name)

src_dirs := $(program_dir) $(hal_dir) $(arch_dir)/$(ARCH) $(od_dir) $(ti_dir)
includes += $(program_dir) $(lua_dir) $(lodepng_dir) $(miniz_dir) $(libs_dir)/SDL_FontCache
includes += $(program_name)/od/glue
includes += $(libs_dir)/rtaudio


# Optional external FFTW staging root with include/ and lib/ subdirs.
# Example: FFTW_STAGE_ROOT=$(PWD)/testing/linux/fftw3/usr
FFTW_STAGE_ROOT ?=

libraries :=
libraries += $(libs_build_dir)/lib$(lua_name).a
libraries += $(libs_build_dir)/liblodepng.a
libraries += $(libs_build_dir)/libminiz.a

# Recursive search for source files
cpp_sources := $(foreach D,$(src_dirs),$(call rwildcard,$D,*.cpp)) 
c_sources := $(foreach D,$(src_dirs),$(call rwildcard,$D,*.c)) 

c_sources := $(filter-out $(program_name)/hal/fft_stub.c,$(c_sources))
c_sources := $(filter-out $(program_name)/olive.c,$(c_sources))

objects := $(addprefix $(out_dir)/,$(c_sources:%.c=%.o) $(cpp_sources:%.cpp=%.o)) 

# Manually add objects 
objects += $(out_dir)/od/glue/app_swig.o
objects += $(out_dir)/libs/SDL_FontCache/SDL_FontCache.o
objects += $(out_dir)/libs/rtaudio/RtAudio.o
objects += $(out_dir)/libs/rtaudio/rtaudio_c.o

# RtAudio compiler selection:
# - Native darwin: clang++ is required for CoreAudio block syntax.
# - Cross builds: use the configured cross C++ compiler.
RTAUDIO_CPP := $(CPP)
RTAUDIO_CFLAGS = $(CFLAGS)
RTAUDIO_EXTRA_CXXFLAGS :=

ifeq ($(ARCH),darwin)
ifneq ($(CROSS_COMPILE),1)
RTAUDIO_CPP := clang++
RTAUDIO_CFLAGS = $(filter-out -rdynamic,$(CFLAGS))
endif
endif

ifneq (,$(findstring clang,$(RTAUDIO_CPP)))
RTAUDIO_EXTRA_CXXFLAGS += -Wno-vla-cxx-extension
endif

$(out_dir)/libs/rtaudio/RtAudio.o: libs/rtaudio/RtAudio.cpp
	@mkdir -p $(@D)
	@$(RTAUDIO_CPP) $(DEPFLAGS) $(RTAUDIO_CFLAGS) -std=gnu++17 $(RTAUDIO_EXTRA_CXXFLAGS) -c $< -o $@

$(out_dir)/libs/rtaudio/rtaudio_c.o: libs/rtaudio/rtaudio_c.cpp
	@mkdir -p $(@D)
	@$(RTAUDIO_CPP) $(DEPFLAGS) $(RTAUDIO_CFLAGS) -std=gnu++17 $(RTAUDIO_EXTRA_CXXFLAGS) -c $< -o $@

ifeq ($(ARCH),linux)
LFLAGS += -Wl,--export-dynamic -Wl,--gc-sections
endif

ifeq ($(shell uname -s),Darwin)
ifneq ($(CROSS_COMPILE),1)
ifeq ($(shell uname -m),arm64)
ARCH_FLAGS=-march=armv8.2-a
else
ARCH_FLAGS=-march=native
endif

# Locate our deps using brew
sdl2 := $(shell brew --prefix sdl2)
sdl2_ttf := $(shell brew --prefix sdl2_ttf)
fftw := $(shell brew --prefix fftw)

CFLAGS += -rdynamic
CFLAGS += -I$(sdl2)/include -I$(sdl2)/include/SDL2 -I$(sdl2_ttf)/include -I$(fftw)/include
CFLAGS += -D__MACOSX_CORE__
CFLAGS += $(ARCH_FLAGS)
LFLAGS += -L$(sdl2)/lib -L$(sdl2_ttf)/lib -L$(fftw)/lib
LFLAGS += -framework CoreAudio -framework CoreFoundation
endif
endif

ifneq ($(FFTW_STAGE_ROOT),)
CFLAGS += -I$(FFTW_STAGE_ROOT)/include
LFLAGS += -L$(FFTW_STAGE_ROOT)/lib
ifeq ($(CROSS_COMPILE),1)
LFLAGS += -Wl,-rpath-link,$(FFTW_STAGE_ROOT)/lib
endif
endif

ifeq ($(CROSS_COMPILE),1)
symbols += TARGET_SSP
CFLAGS += -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/SDL2
CFLAGS += -D__LINUX_ALSA__
LFLAGS += -L$(SYSROOT)/usr/lib -Wl,-rpath-link,$(SYSROOT)/usr/lib
LFLAGS += -lasound
endif

CFLAGS += -DFIRMWARE_VERSION=\"$(FIRMWARE_VERSION)\"
CFLAGS += -DBUILD_PROFILE=\"$(PROFILE)\"
LFLAGS += -lSDL2 -lSDL2_ttf
LFLAGS += -lfftw3f
ifeq ($(ARCH),darwin)
ifneq ($(CROSS_COMPILE),1)
LFLAGS += -lc++ -lc++abi
endif
endif
LFLAGS += -lpthread
LFLAGS += -lm -ldl -lstdc++ 

all: $(out_dir)/$(program_name).elf

$(objects): scripts/env.mk scripts/ssp.mk

$(out_dir)/$(program_name).elf: $(objects) $(libraries)
	@mkdir -p $(@D)	
	@echo $(describe_env) LINK $(describe_target)
	@$(CC) $(CFLAGS) -o $@ $(objects) $(libraries) $(LFLAGS)

clean:
	rm -rf $(out_dir)

addr2line: $(out_dir)/$(program_name).elf
	@echo $(describe_env) Find ${ADDRESS} in $(out_dir)/$(program_name).elf
	@$(ADDR2LINE) -p -f -i -C -e $(out_dir)/$(program_name).elf -a $(ADDRESS)

missing: $(objects) $(libraries)
	@echo $(describe_env) Generating list of missing references...
	-@$(CC) $(CFLAGS) -o $@ $(objects) $(libraries) $(LFLAGS) 2> $(out_dir)/error.log
	@$(PYTHON) list-undefined.py $(out_dir)/error.log > $(out_dir)/missing.log	

include scripts/rules.mk
