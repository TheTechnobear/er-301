# Build Tools for Linux
ifneq ($(CROSS_COMPILE_EMU),1)

CC := gcc -fdiagnostics-color -fmax-errors=5
CPP := g++ -fdiagnostics-color -fmax-errors=5
OBJCOPY := objcopy
OBJDUMP := objdump
ADDR2LINE := addr2line
LD := gcc -fdiagnostics-color
AR := gcc-ar
SIZE := size
STRIP := strip
READELF := readelf
NM := nm
SWIG := swig
PYTHON := python3
ZIP := zip

else

# Cross-compile toolchain for emu on Linux/arm from a non-Linux host.
ifneq ($(SSP_BUILDROOT),)
BUILDROOT ?= $(SSP_BUILDROOT)
endif

ifndef BUILDROOT
$(error CROSS_COMPILE_EMU=1 requires BUILDROOT (or SSP_BUILDROOT) to be set)
endif

ifndef TOOLSROOT
ifeq ($(shell uname -s),Darwin)
ifeq ($(shell uname -m),arm64)
TOOLSROOT := /opt/homebrew/opt/llvm/bin
else
TOOLSROOT := /usr/local/opt/llvm/bin
endif
else
TOOLSROOT := /usr/bin
endif
endif

TRIPLE ?= arm-linux-gnueabihf
SYSROOT ?= $(BUILDROOT)/arm-rockchip-linux-gnueabihf/sysroot
GCCROOT ?= $(BUILDROOT)/lib/gcc/arm-rockchip-linux-gnueabihf/8.4.0
GXXROOT ?= $(BUILDROOT)/arm-rockchip-linux-gnueabihf/include/c++/8.4.0
CROSS_TOOL_FLAGS := --target=$(TRIPLE) --sysroot=$(SYSROOT)

CC := $(TOOLSROOT)/clang $(CROSS_TOOL_FLAGS) -fdiagnostics-color -fmax-errors=5
CPP := $(TOOLSROOT)/clang++ $(CROSS_TOOL_FLAGS) -fdiagnostics-color -fmax-errors=5
OBJCOPY := $(TOOLSROOT)/llvm-objcopy
OBJDUMP := $(TOOLSROOT)/llvm-objdump
ADDR2LINE := $(TOOLSROOT)/llvm-addr2line
LD := $(TOOLSROOT)/clang $(CROSS_TOOL_FLAGS) -fdiagnostics-color -fuse-ld=lld
AR := $(TOOLSROOT)/llvm-ar
SIZE := $(TOOLSROOT)/llvm-size
STRIP := $(TOOLSROOT)/llvm-strip
READELF := $(TOOLSROOT)/llvm-readelf
NM := $(TOOLSROOT)/llvm-nm
SWIG := swig
PYTHON := python3
ZIP := zip

export PKG_CONFIG_DIR :=
export PKG_CONFIG_LIBDIR := $(SYSROOT)/usr/lib/pkgconfig:$(SYSROOT)/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR := $(SYSROOT)

LFLAGS += -L$(SYSROOT)/lib -B$(SYSROOT)/lib
LFLAGS += -Wl,-rpath-link,$(SYSROOT)/lib
LFLAGS += -L$(GCCROOT) -B$(GCCROOT)
LFLAGS += -Wl,-rpath-link,$(GCCROOT)

# Match libstdc++ include paths used in xcSSP.cmake.
CFLAGS += -I$(GXXROOT)
CFLAGS += -I$(GXXROOT)/arm-rockchip-linux-gnueabihf

$(info using TOOLSROOT  :  $(TOOLSROOT))
$(info using BUILDROOT  :  $(BUILDROOT))
$(info using SYSROOT    :  $(SYSROOT))

endif