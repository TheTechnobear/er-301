PERCUSSA_TOOLCHAIN_NAME := ssp
PERCUSSA_TOOLCHAIN_PANEL := ssp
PERCUSSA_PANEL ?= ssp
LIBS_BUILD_FLAVOR := ssp
FFTW_STAGE_ROOT ?= $(CURDIR)/testing/linux/fftw3-ssp/usr

ARCH := linux
CROSS_COMPILE := 1

BUILDROOT ?= $(SSP_BUILDROOT)
ifeq ($(strip $(BUILDROOT)),)
$(error SSP toolchain requires BUILDROOT or SSP_BUILDROOT.)
endif

TRIPLE := arm-linux-gnueabihf
SYSROOT := $(BUILDROOT)/arm-rockchip-linux-gnueabihf/sysroot
GCCROOT := $(BUILDROOT)/lib/gcc/arm-rockchip-linux-gnueabihf/8.4.0
GXXROOT := $(BUILDROOT)/arm-rockchip-linux-gnueabihf/include/c++/8.4.0
GXX_INCLUDE_TRIPLE := arm-rockchip-linux-gnueabihf
LINUX_CROSS_CPUFLAGS := -mcpu=cortex-a17 -mfloat-abi=hard -mfpu=neon-vfpv4