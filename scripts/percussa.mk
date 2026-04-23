include scripts/env.mk
include scripts/utils.mk

.DEFAULT_GOAL := all

program_name := percussa
program_dir := $(program_name)

src_dirs := $(program_dir) $(hal_dir) $(arch_dir)/$(ARCH) $(od_dir) $(ti_dir)
includes += $(program_dir) $(lua_dir) $(lodepng_dir) $(miniz_dir) $(libs_dir)/SDL_FontCache
includes += $(program_name)/od/glue
includes += $(libs_dir)/rtaudio

ifeq ($(ARCH),darwin)
PERCUSSA_PLATFORM ?= host-sdl
else
PERCUSSA_PLATFORM ?= fbdev
endif

PERCUSSA_PANEL ?= ssp
program_variant := $(program_name)-$(PERCUSSA_PANEL)
out_dir := $(build_dir)/$(program_variant)

symbols += BUILDOPT_LUA_USE_REALLOC

ifeq ($(PERCUSSA_PANEL),ssp)
symbols += PERCUSSA_PANEL_SSP TARGET_SSP
else ifeq ($(PERCUSSA_PANEL),xmx)
symbols += PERCUSSA_PANEL_XMX TARGET_XMX
else
$(error Unsupported PERCUSSA_PANEL '$(PERCUSSA_PANEL)'. Use ssp or xmx.)
endif

ifeq ($(PERCUSSA_PLATFORM),host-sdl)
percussa_uses_sdl := 1
symbols += PERCUSSA_PLATFORM_HOST_SDL
else ifeq ($(PERCUSSA_PLATFORM),fbdev)
percussa_uses_sdl := 0
symbols += PERCUSSA_PLATFORM_FBDEV
else ifeq ($(PERCUSSA_PLATFORM),plugin)
percussa_uses_sdl := 0
symbols += PERCUSSA_PLATFORM_PLUGIN
else
$(error Unsupported PERCUSSA_PLATFORM '$(PERCUSSA_PLATFORM)'. Use host-sdl, fbdev, or plugin.)
endif

# Optional external FFTW staging root with include/ and lib/ subdirs.
# Example: FFTW_STAGE_ROOT=$(PWD)/testing/linux/fftw3/usr
FFTW_STAGE_ROOT ?=

libraries :=
libraries += $(libs_build_dir)/lib$(lua_name).a
libraries += $(libs_build_dir)/liblodepng.a
libraries += $(libs_build_dir)/libminiz.a

all_cpp_sources := $(foreach D,$(src_dirs),$(call rwildcard,$D,*.cpp))
all_c_sources := $(foreach D,$(src_dirs),$(call rwildcard,$D,*.c))

# Compile the same broad surface as ssp.mk, but prefer percussa-local files
# where that migration already exists.
all_cpp_sources := $(filter-out \
	$(hal_dir)/events.cpp \
	$(hal_dir)/pump/pump.cpp \
	$(od_dir)/glue/Interpreter.cpp, \
	$(all_cpp_sources))

all_c_sources := $(filter-out \
	$(arch_dir)/$(ARCH)/hal/fileops.c \
	$(hal_dir)/simd.c \
	$(hal_dir)/pump/pidcontrol.c \
	$(hal_dir)/pump/rfifo4.c \
	$(hal_dir)/pump/resample4.c \
	$(od_dir)/config.c, \
	$(all_c_sources))

panel_cpp_sources := $(program_dir)/panel/Family.cpp
panel_cpp_sources += $(program_dir)/hal/card.cpp
panel_cpp_sources += $(program_dir)/hal/concurrency/Mutex.cpp
panel_cpp_sources += $(program_dir)/hal/usb.cpp
panel_cpp_sources += $(program_dir)/hal/pump/pump.cpp
panel_cpp_sources += $(program_dir)/runtime/ssp/CardState.cpp
panel_c_sources := $(program_dir)/od/config.c
panel_c_sources += $(program_dir)/hal/simd.c
panel_c_sources += $(program_dir)/hal/pump/pidcontrol.c
panel_c_sources += $(program_dir)/hal/pump/rfifo4.c
panel_c_sources += $(program_dir)/hal/pump/resample4.c
ifeq ($(PERCUSSA_PANEL),ssp)
panel_cpp_sources += $(program_dir)/panel/ssp/SspController.cpp
panel_cpp_sources += $(program_dir)/panel/ssp/SspFrontPanelState.cpp
panel_cpp_sources += $(program_dir)/panel/ssp/SspPanel.cpp
panel_cpp_sources += $(program_dir)/runtime/ssp/SspBootstrap.cpp
panel_cpp_sources += $(program_dir)/runtime/ssp/LegacyHardware.cpp
panel_cpp_sources += $(program_dir)/ssp/CommandLine.cpp
panel_cpp_sources += $(program_dir)/ssp/KeyValueStore.cpp
else ifeq ($(PERCUSSA_PANEL),xmx)
panel_cpp_sources += $(program_dir)/panel/xmx/XmxController.cpp
panel_cpp_sources += $(program_dir)/panel/xmx/XmxPanel.cpp
endif

platform_cpp_sources :=
ifeq ($(PERCUSSA_PLATFORM),host-sdl)
platform_cpp_sources += $(program_dir)/platform/HostSdlPlatform.cpp
else ifeq ($(PERCUSSA_PLATFORM),fbdev)
platform_cpp_sources += $(program_dir)/platform/FbdevPlatform.cpp
else ifeq ($(PERCUSSA_PLATFORM),plugin)
platform_cpp_sources += $(program_dir)/platform/PluginPlatform.cpp
endif

common_cpp_sources := $(filter-out \
	$(program_dir)/hal/card.cpp \
	$(program_dir)/hal/concurrency/Mutex.cpp \
	$(program_dir)/hal/usb.cpp \
	$(program_dir)/hal/pump/pump.cpp \
	$(program_dir)/runtime/ssp/CardState.cpp \
	$(program_dir)/panel/Family.cpp \
	$(program_dir)/panel/ssp/SspController.cpp \
	$(program_dir)/panel/ssp/SspFrontPanelState.cpp \
	$(program_dir)/panel/ssp/SspPanel.cpp \
	$(program_dir)/runtime/ssp/SspBootstrap.cpp \
	$(program_dir)/runtime/ssp/LegacyHardware.cpp \
	$(program_dir)/ssp/CommandLine.cpp \
	$(program_dir)/ssp/KeyValueStore.cpp \
	$(program_dir)/panel/xmx/XmxController.cpp \
	$(program_dir)/panel/xmx/XmxPanel.cpp \
	$(program_dir)/platform/HostSdlPlatform.cpp \
	$(program_dir)/platform/FbdevPlatform.cpp \
	$(program_dir)/platform/PluginPlatform.cpp, \
	$(all_cpp_sources))

cpp_sources := $(common_cpp_sources) $(panel_cpp_sources) $(platform_cpp_sources)
c_sources := $(filter-out \
	$(program_dir)/od/config.c \
	$(program_dir)/hal/simd.c \
	$(program_dir)/hal/pump/pidcontrol.c \
	$(program_dir)/hal/pump/rfifo4.c \
	$(program_dir)/hal/pump/resample4.c, \
	$(all_c_sources)) $(panel_c_sources)

objects := $(addprefix $(out_dir)/,$(c_sources:%.c=%.o) $(cpp_sources:%.cpp=%.o))
objects += $(out_dir)/od/glue/app_swig.o
ifeq ($(percussa_uses_sdl),1)
objects += $(out_dir)/libs/SDL_FontCache/SDL_FontCache.o
endif
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
ifeq ($(percussa_uses_sdl),1)
CFLAGS += -I$(sdl2)/include -I$(sdl2)/include/SDL2 -I$(sdl2_ttf)/include -I$(fftw)/include
else
CFLAGS += -I$(fftw)/include
endif
CFLAGS += -D__MACOSX_CORE__
CFLAGS += $(ARCH_FLAGS)
ifeq ($(percussa_uses_sdl),1)
LFLAGS += -L$(sdl2)/lib -L$(sdl2_ttf)/lib -L$(fftw)/lib
else
LFLAGS += -L$(fftw)/lib
endif
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
CFLAGS += -D_GNU_SOURCE
CFLAGS += -I$(SYSROOT)/usr/include
ifeq ($(percussa_uses_sdl),1)
CFLAGS += -I$(SYSROOT)/usr/include/SDL2
endif
CFLAGS += -D__LINUX_ALSA__
LFLAGS += -L$(SYSROOT)/usr/lib -Wl,-rpath-link,$(SYSROOT)/usr/lib
LFLAGS += -lasound
endif

CFLAGS += -DFIRMWARE_VERSION=\"$(FIRMWARE_VERSION)\"
CFLAGS += -DBUILD_PROFILE=\"$(PROFILE)\"
ifeq ($(percussa_uses_sdl),1)
LFLAGS += -lSDL2 -lSDL2_ttf
endif
LFLAGS += -lfftw3f
ifeq ($(ARCH),darwin)
ifneq ($(CROSS_COMPILE),1)
LFLAGS += -lc++ -lc++abi
endif
endif
LFLAGS += -lpthread
LFLAGS += -lm -ldl -lstdc++

all: $(out_dir)/$(program_variant).elf

$(objects): scripts/env.mk scripts/percussa.mk

$(out_dir)/$(program_variant).elf: $(objects) $(libraries)
	@mkdir -p $(@D)
	@echo $(describe_env) LINK $(describe_target)
	@$(CC) $(CFLAGS) -o $@ $(objects) $(libraries) $(LFLAGS)

clean:
	rm -rf $(out_dir)

addr2line: $(out_dir)/$(program_variant).elf
	@echo $(describe_env) Find ${ADDRESS} in $(out_dir)/$(program_variant).elf
	@$(ADDR2LINE) -p -f -i -C -e $(out_dir)/$(program_variant).elf -a $(ADDRESS)

missing: $(objects) $(libraries)
	@echo $(describe_env) Generating list of missing references...
	-@$(CC) $(CFLAGS) -o $@ $(objects) $(libraries) $(LFLAGS) 2> $(out_dir)/error.log
	@$(PYTHON) list-undefined.py $(out_dir)/error.log > $(out_dir)/missing.log

include scripts/rules.mk
