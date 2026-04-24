PERCUSSA_TOOLCHAIN_NAME := xmx
PERCUSSA_TOOLCHAIN_PANEL := xmx
PERCUSSA_PANEL ?= xmx
LIBS_BUILD_FLAVOR := xmx
FFTW_STAGE_ROOT ?= $(CURDIR)/testing/linux/fftw3-xmx/usr

ARCH := linux
CROSS_COMPILE := 1

BUILDROOT ?= $(XMX_BUILDROOT)
ifeq ($(strip $(BUILDROOT)),)
$(error XMX toolchain requires BUILDROOT or XMX_BUILDROOT.)
endif

TRIPLE := aarch64-rockchip-linux-gnu
SYSROOT := $(BUILDROOT)/aarch64-rockchip-linux-gnu/sysroot
GCCROOT := $(BUILDROOT)/lib/gcc/aarch64-rockchip-linux-gnu/8.4.0
GXXROOT := $(BUILDROOT)/aarch64-rockchip-linux-gnu/include/c++/8.4.0
GXX_INCLUDE_TRIPLE := aarch64-rockchip-linux-gnu
LINUX_CROSS_CPUFLAGS := -mcpu=cortex-a35