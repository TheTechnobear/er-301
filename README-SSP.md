# ER-301 SSP Build Notes

This document describes how to cross-compile the ER-301 emulator (`emu`) from macOS (Apple Silicon) to Linux/ARM using the SSP SDK/buildroot toolchain.

## Table of Contents

- [Scope](#scope)
- [Host and Target](#host-and-target)
- [Toolchain Values](#toolchain-values)
- [Prerequisites](#prerequisites)
- [Environment](#environment)
- [Git Tags and Build Versions](#git-tags-and-build-versions)
- [Quick Start (Normal Flow)](#quick-start-normal-flow)
- [Advanced Build Options](#advanced-build-options)
- [Development Notes](#development-notes)
  - [Project-Local FFTW Staging](#project-local-fftw-staging)
  - [Runtime Libraries (Reference)](#runtime-libraries-reference)
- [Font Configuration](#font-configuration)
- [Troubleshooting](#troubleshooting)
  - [Error: `CROSS_COMPILE=1 requires BUILDROOT`](#error-cross_compile1-requires-buildroot)
  - [Missing SDL2/SDL2_ttf/fftw during link](#missing-sdl2sdl2_ttffftw-during-link)
  - [Missing fftw3.h during compile](#missing-fftw3h-during-compile)
  - [Status (2026-04-18): Cross-Build Working](#status-2026-04-18-cross-build-working)
  - [LLVM tools not found](#llvm-tools-not-found)
- [3rd Party Modules](#3rd-party-modules)
  - [er301 sdk](#er301-sdk)
  - [Updating tutorial.mk-style Module Makefiles](#updating-tutorialmk-style-module-makefiles)

## Scope

- Target covered here: emulator (`emu`) and modules (`core`) for SSP only
- Hardware firmware targets (`am335x`, `firmware`, etc.) are not covered.

see `./README.md` for more general details
see `./ssp/SSP.md` for the SSP specific UI and integration implmentation

## Host and Target

- Host: macOS arm64 (Apple M1/M2/M3)
- Target: Linux armv7 hard-float (`arm-linux-gnueabihf`)

## Toolchain Values

The makefiles use the same cross values as `xcSSP.cmake`:

- `BUILDROOT`
- `TOOLSROOT` (defaults to Homebrew LLVM on macOS arm64)
- `SYSROOT = $(BUILDROOT)/arm-rockchip-linux-gnueabihf/sysroot`
- `GCCROOT = $(BUILDROOT)/lib/gcc/arm-rockchip-linux-gnueabihf/8.4.0`
- `TRIPLE = arm-linux-gnueabihf`

## Prerequisites

1. SSP SDK/buildroot is installed and accessible.
2. LLVM toolchain is installed on host macOS.
   - Default expected path on Apple Silicon: `/opt/homebrew/opt/llvm/bin`
	- Install LLVM and lld with Homebrew: `brew install llvm lld`
3. Target sysroot contains `SDL2` and `SDL2_ttf` (dynamic `.so` libs).
4. FFTW is provided project-locally via `testing/linux/fftw3/` (see [Project-Local FFTW Staging](#project-local-fftw-staging)).
5. A suitable font is present on the target device (e.g. `LiberationSans-Regular.ttf`; see [Font Configuration](#font-configuration)).

## Environment

For cross builds, set:

```bash
export BUILDROOT=/path/to/your/arm-rockchip-linux-gnueabihf_sdk-buildroot
```

Notes:

- Cross auto-detection now uses `BUILDROOT` only.
- Keeping `SSP_BUILDROOT` set does not force cross mode.
- To cross-build using an existing `SSP_BUILDROOT`, run: `export BUILDROOT=$SSP_BUILDROOT`.
-  the xcSSP.cmake-ref-only was used as a reference that I use with cmake on other projects, it servers no purpose here

Optional overrides:

```bash
export TOOLSROOT=/opt/homebrew/opt/llvm/bin
export TRIPLE=arm-linux-gnueabihf
```

## Git Tags and Build Versions

Build version strings are derived from Git tags. If matching tags are missing in your local clone, versioned outputs can become empty (for example, `core-.pkg`).

This affects both:

- Firmware archive naming (for example, `er-301-v...zip`)
- Mod package naming (for example, `core-...pkg`, `teletype-...pkg`)

If you are working in a fork, add the canonical upstream and fetch tags:

```bash
git remote add upstream https://github.com/odevices/er-301
git fetch upstream --tags
```

If `upstream` already exists:

```bash
git remote set-url upstream https://github.com/odevices/er-301
git fetch upstream --tags
```

Verify tag visibility and version resolution:

```bash
git tag -l "v*.*.*-*"
git describe --match "v*.*.*-*" --tags --abbrev=0
```

Note: the `upstream` remote is local Git config in your clone. Other developers must add it in their own clones if they also need upstream tags.

## Quick Start (Normal Flow)

This is the default path for day-to-day use.

### 1. Set BUILDROOT and build fftw3

```bash
export BUILDROOT=/path/to/your/arm-rockchip-linux-gnueabihf_sdk-buildroot
DESTDIR=$PWD/testing/linux/fftw3 ./scripts/build-fftw-cross.sh
```

### 2. Build emulator

```bash
make emu
```

### 3. Build core package

```bash
make core
```

### 4. Deploy and run on SSP

Copy the emulator binary:

```bash
scp -O testing/linux/emu/emu.elf root@192.168.0.150:/media/BOOT/er301/er301.elf
```

Copy the Lua root:

```bash
scp -O -r xroot root@192.168.0.150:/media/BOOT/er301/
```

Copy required shared libraries:

```bash
scp -O libSDL2-2.0.so.0 libSDL2_ttf-2.0.so.0 root@192.168.0.150:/media/BOOT/er301/
```

Copy the core package to rear for auto-install on next boot:

```bash
scp -O testing/linux/mods/core-*.pkg root@192.168.0.150:/media/BOOT/er301/rear
```

Create `/media/BOOT/er301/emu.config` on target:

```text
XROOT /media/BOOT/er301/xroot
REAR_ROOT /media/BOOT/er301/rear
FRONT_ROOT /media/BOOT/er301/front
SESSION /media/BOOT/er301/emu.session
```

Run:

```bash
LD_LIBRARY_PATH=/media/BOOT/er301 /media/BOOT/er301/er301.elf -c /media/BOOT/er301/emu.config
```

Notes:

- With `BUILDROOT` set, `make emu` auto-selects Linux cross mode (`ARCH=linux`, `CROSS_COMPILE=1`).
- FFTW uses staged files from `testing/linux/fftw3/usr`.

## Advanced Build Options

Build without FFTW:

```bash
make emu WITH_FFTW_EMU=0
```

Override FFTW staging path:

```bash
make emu FFTW_STAGE_ROOT=/custom/path/usr
```

Clean artifacts:

```bash
make emu-clean
```

Alternative low-level invocation:

```bash
make ARCH=linux CROSS_COMPILE=1 -f scripts/lua.mk
make ARCH=linux CROSS_COMPILE=1 -f scripts/miniz.mk
make ARCH=linux CROSS_COMPILE=1 -f scripts/lodepng.mk
make ARCH=linux CROSS_COMPILE=1 -f scripts/emu.mk
```

Output layout:

- `testing/linux/emu/emu.elf` (default profile: `testing`)
- `testing/linux/libs/` (cross-built static libs)

For other profiles (`debug`, `release`), the first path segment changes accordingly.

## Development Notes

When `CROSS_COMPILE=1`:

- Compiler switches to clang/clang++ cross invocation with `--target` and `--sysroot`.
- Linker uses `lld` with sysroot and gcc runtime search paths.
- `PKG_CONFIG_*` environment variables are set for sysroot-aware dependency resolution.
- Linux emulator CFLAGS switch from x86 (`-msse4`) to ARM (`-mcpu=cortex-a17 -mfloat-abi=hard -mfpu=neon-vfpv4`).
- Emulator include/library paths add `$(SYSROOT)/usr/include`, `$(SYSROOT)/usr/include/SDL2`, and `$(SYSROOT)/usr/lib`.
- `FFTW_STAGE_ROOT=/path/to/stage/usr` adds `-I.../include` and `-L.../lib` for FFTW without touching sysroot.

### Project-Local FFTW Staging

The SSP sysroot does not include FFTW. This is a one-time manual step before the first cross build.

`scripts/build-fftw-cross.sh` downloads FFTW 3.3.10 source, cross-configures it for ARM, and installs a static library into a project-local staging directory. It does not modify the sysroot.

```bash
export BUILDROOT=/path/to/arm-rockchip-linux-gnueabihf_sdk-buildroot
DESTDIR=$PWD/testing/linux/fftw3 ./scripts/build-fftw-cross.sh
```

Key environment variables (all optional, with defaults):

| Variable | Default | Description |
|---|---|---|
| `BUILDROOT` | — | SDK root; used to locate `SYSROOT` and `GCCROOT` |
| `TOOLSROOT` | `/opt/homebrew/opt/llvm/bin` | Directory containing `clang`, `llvm-ar`, etc. |
| `DESTDIR` | `$REPO_ROOT/.local/fftw-stage` | Installation destination |
| `FFTW_VERSION` | `3.3.10` | FFTW release to download |
| `JOBS` | host CPU count | Parallel build jobs |

The script is idempotent: it skips re-downloading the tarball and re-running `configure` if stamps exist. To force a clean reconfigure, delete `.build/fftw-cross/stamps/`.

Expected outputs:

- `$DESTDIR/usr/include/fftw3.h`
- `$DESTDIR/usr/lib/libfftw3f.a`

The top-level make target uses `FFTW_STAGE_ROOT=$(CURDIR)/testing/linux/fftw3/usr` by default, so no extra flags are needed after staging.

FFTW links statically and does not need to be present on the target device at runtime.

### Runtime Libraries (Reference)

The binary requires `libSDL2` and `libSDL2_ttf` on target. Other dependencies (`libm`, `libdl`, `libstdc++`, `libgcc_s`, `libc`, `libpthread`, `libfreetype`, `libpng16`, `libz`, `libbz2`) are typically part of the standard SSP rootfs.

Reference `ldd` output on target:

```text
[root@rockchip:/media/BOOT/er301]# ldd er301.elf
	linux-vdso.so.1 (0xbea87000)
	libSDL2-2.0.so.0 => /media/BOOT/er301/libSDL2-2.0.so.0 (0xb69c4000)
	libSDL2_ttf-2.0.so.0 => /media/BOOT/er301/libSDL2_ttf-2.0.so.0 (0xb6988000)
	libm.so.6 => /lib/libm.so.6 (0xb691c000)
	libdl.so.2 => /lib/libdl.so.2 (0xb6909000)
	libstdc++.so.6 => /usr/lib/libstdc++.so.6 (0xb67b0000)
	libgcc_s.so.1 => /lib/libgcc_s.so.1 (0xb6780000)
	libc.so.6 => /lib/libc.so.6 (0xb6631000)
	libpthread.so.0 => /lib/libpthread.so.0 (0xb6607000)
	libfreetype.so.6 => /usr/lib/libfreetype.so.6 (0xb656a000)
	/lib/ld-linux-armhf.so.3 (0xb6f69000)
	libbz2.so.1.0 => /usr/lib/libbz2.so.1.0 (0xb654a000)
	libpng16.so.16 => /usr/lib/libpng16.so.16 (0xb650e000)
	libz.so.1 => /usr/lib/libz.so.1 (0xb64e9000)
```


## Font Configuration

The emulator searches for a TTF font at runtime in this order:

1. `/usr/share/fonts/truetype/freefont/FreeSans.ttf`
2. `libs/SDL_FontCache/test/fonts/FreeSans.ttf`
3. `/usr/share/fonts/liberation/LiberationSans-Regular.ttf`
4. `/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf`

Failed attempts log `INFO: Unable to open file...` — these are harmless. The first path that succeeds is used.

On the SSP device (which typically lacks FreeSans), Liberation Sans is expected at `/usr/share/fonts/liberation/LiberationSans-Regular.ttf`. Install the `liberation-fonts` package if not present.

## Troubleshooting

### Error: `CROSS_COMPILE=1 requires BUILDROOT`

Set `BUILDROOT` before running cross builds.

Examples:

```bash
export BUILDROOT=/path/to/arm-rockchip-linux-gnueabihf_sdk-buildroot
make emu
```

or, if you keep only `SSP_BUILDROOT` exported:

```bash
export BUILDROOT=$SSP_BUILDROOT
make emu
```

### Missing SDL2/SDL2_ttf/fftw during link

Confirm the target sysroot includes:

- `usr/include/SDL2`
- `usr/lib/libSDL2*`
- `usr/lib/libSDL2_ttf*`

Also confirm FFTW is staged locally and visible via `FFTW_STAGE_ROOT`:

- `testing/linux/fftw3/usr/include/fftw3.h`
- `testing/linux/fftw3/usr/lib/libfftw3f.a`

### Missing fftw3.h during compile

If build stops in `emu/hal/fft.c` with:

- `fatal error: 'fftw3.h' file not found`

then your target sysroot is missing FFTW headers for the target architecture.

### Status (2026-04-18): Cross-Build Working

The cross emulator build is fully functional as of this date:

- `lua`, `miniz`, `lodepng`, and `emu` all cross-compile successfully.
- Output: `testing/linux/emu/emu.elf` — ARM 32-bit ELF.
- FFTW resolved via project-local staging (`scripts/build-fftw-cross.sh`).
- SDL2_ttf resolved by the user compiling it into the sysroot.
- Font resolved via Liberation Sans fallback in `emu/Window.cpp`.

**Runtime shared library requirements on the target device:**

| Library | Notes |
|---|---|
| `libSDL2-2.0.so.0` | Required |
| `libSDL2_ttf-2.0.so.0` | Required |
| `libm.so.6` | Standard C math |
| `libdl.so.2` | Dynamic linking |
| `libstdc++.so.6` | C++ stdlib |
| `libgcc_s.so.1` | GCC runtime |
| `libc.so.6` | Standard C |
| `libfftw3f` | **Static** — not needed on device |

All other libs are statically linked or part of the standard buildroot rootfs.

### LLVM tools not found

Set:

```bash
export TOOLSROOT=/opt/homebrew/opt/llvm/bin
```

or your local LLVM install path.

If FFTW configure fails with `invalid linker name in argument '-fuse-ld=...'`, install lld:

```bash
brew install lld
```

Then re-run:

```bash
DESTDIR=$PWD/testing/linux/fftw3 ./scripts/build-fftw-cross.sh
```
## 3rd Party Modules

if you wish to compile a 3rd party module a number of changes will need to be made.
I cannot detail exactly what changes are required, as it depends on how the developer used the er301 SDK, and what changes they made exactly.
but below I detail changes, that are likely, esp if they followed the er301 module tutorial guide.

### er301 sdk
so they will likely have used the er301 repo, since it contains headers and makefile scripts need to build.
either by asking you to downlaod the sdk, a point SDKPATH to it, 
or include er301 sdk as a submodule, if so you will need to replace that , with this repo.
in both cases, you simply need to replace the er301 repo with this one.


### Updating tutorial.mk-style Module Makefiles

3rd-party modules may use a local copy of `tutorial.mk` as their build wrapper.
we will need to updates these to allow for cross-compilation.

the actual cross-compiler is setup in linux.mk, which it likely will include
however, they will likely use a wrapper, similar to the SDKs tutorial.mk or env.mk.

this is where we setup the CROSS_COMPILE flag used by linux.mk, and some other settings.
below, are details of the changes, based on what was done for env.mk, emu.mk and tutorial.mk.

1. auto switching to cross compile and linux
change architure detection, see how we force to linux if BUILDROOT env is set

```Makefile
ifndef ARCH
	# If a Buildroot SDK path is provided explicitly, assume linux emu cross-compile.
	ifneq ($(strip $(BUILDROOT)),)
		ARCH = linux
	else
  SYSTEM_NAME := $(shell uname -s)
  ifeq ($(SYSTEM_NAME),Linux)
    ARCH = linux
  else ifeq ($(SYSTEM_NAME),Darwin)
    ARCH = darwin
  else
    $(error Unsupported system $(SYSTEM_NAME))
  endif
	endif
endif
```

2. In the Linux block, (`ifeq ($(ARCH),linux)`),

a) add `CROSS_COMPILE ?= auto` and resolve `auto` to `1` when `BUILDROOT` is set, else `0`.

```makefile
CROSS_COMPILE ?= auto

ifeq ($(CROSS_COMPILE),auto)
ifneq ($(BUILDROOT),)
CROSS_COMPILE := 1
else
CROSS_COMPILE := 0
endif
endif
```
b)  set CFLAGS conditionallya and  add `EMU_CROSS_COMPILE` to symbols.

```Makefile
ifeq ($(CROSS_COMPILE),1)
symbols += EMU_CROSS_COMPILE
CFLAGS.linux = -Wno-deprecated-declarations -Wno-c++11-narrowing -mcpu=cortex-a17 -mfloat-abi=hard -mfpu=neon-vfpv4 -fPIC
else
CFLAGS.linux = -Wno-deprecated-declarations -msse4 -fPIC
endif
```

c)  finally it will include `scripts/linux.mk` 

3. after, look for the swig compiler flags,
(this may not be required, depends on module)
a) Make SWIG compile flags inherit full `CFLAGS` (size-optimized), so cross sysroot/include additions from `linux.mk` are preserved.
```Makefile
CFLAGS.swig = $(subst $(CFLAGS.speed),$(CFLAGS.size),$(CFLAGS))
CFLAGS.swig += -I$(SDKPATH)/libs/lua54
```

Quick validation:

```bash
make clean ARCH=linux
make ARCH=linux CROSS_COMPILE=1
```

Expected result: versioned `.pkg` output builds without missing standard C/C++ headers.
