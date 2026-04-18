# ER-301 SSP Build Notes

This document describes how to cross-compile the ER-301 emulator (`emu`) from macOS (Apple Silicon) to Linux/ARM using the SSP SDK/buildroot toolchain.

## Scope

- Target covered here: emulator (`emu`) only.
- Hardware firmware targets (`am335x`, `firmware`, etc.) are not covered.

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

Optional overrides:

```bash
export TOOLSROOT=/opt/homebrew/opt/llvm/bin
export TRIPLE=arm-linux-gnueabihf
```

## Build Commands

From repository root:

```bash
make emu-cross
```

Optional (build without FFTW):

```bash
make emu-cross WITH_FFTW_EMU=0
```

The `FFTW_STAGE_ROOT` is baked into the `emu-cross` target (defaults to `testing/linux/fftw3/usr`). Override if staged elsewhere:

```bash
make emu-cross FFTW_STAGE_ROOT=/custom/path/usr
```

This target builds:

1. `lua`
2. `miniz`
3. `lodepng`
4. `emu`

with:

- `ARCH=linux`
- `CROSS_COMPILE=1`

### Clean Cross Build Artifacts

```bash
make emu-cross-clean
```

## Alternative Invocation

If you prefer not to use the top-level shortcut target:

```bash
make ARCH=linux CROSS_COMPILE=1 -f scripts/lua.mk
make ARCH=linux CROSS_COMPILE=1 -f scripts/miniz.mk
make ARCH=linux CROSS_COMPILE=1 -f scripts/lodepng.mk
make ARCH=linux CROSS_COMPILE=1 -f scripts/emu.mk
```

## Output Layout

Cross emulator outputs are generated under:

- `testing/linux/emu/emu.elf` (for default `PROFILE=testing`)
- `testing/linux/libs/` (cross-built static libs)

For other profiles (`debug`, `release`), the first path segment changes accordingly.

## Notes on Build Behavior

When `CROSS_COMPILE=1`:

- Compiler switches to clang/clang++ cross invocation with `--target` and `--sysroot`.
- Linker uses `lld` with sysroot and gcc runtime search paths.
- `PKG_CONFIG_*` environment variables are set for sysroot-aware dependency resolution.
- Linux emulator CFLAGS switch from x86 (`-msse4`) to ARM (`-mcpu=cortex-a17 -mfloat-abi=hard -mfpu=neon-vfpv4`).
- Emulator include/library paths add `$(SYSROOT)/usr/include`, `$(SYSROOT)/usr/include/SDL2`, and `$(SYSROOT)/usr/lib`.
- `WITH_FFTW_EMU=0` switches emu to a stub FFT backend and omits `-lfftw3f`.
- `FFTW_STAGE_ROOT=/path/to/stage/usr` adds `-I.../include` and `-L.../lib` for FFTW without touching sysroot.

## Project-Local FFTW Staging

The SSP sysroot does not include FFTW. This is a **one-time manual step** before the first cross build.

`scripts/build-fftw-cross.sh` downloads the FFTW 3.3.10 source tarball, cross-configures it for ARM, and installs a static library into a project-local staging directory. It does **not** modify the sysroot.

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

The `emu-cross` Makefile target automatically passes `FFTW_STAGE_ROOT=$(CURDIR)/testing/linux/fftw3/usr`, so no extra flags are needed after staging.

FFTW links **statically** — it does not need to be present on the target device at runtime.

## Deploying to SSP

### 1. Copy the binary

```bash
scp testing/linux/emu/emu.elf root@192.168.0.150:/media/BOOT/er301/er301.elf
```

### 2. Copy the Lua root

```bash
scp -r xroot root@192.168.0.150:/media/BOOT/er301/
```

### 3. Shared libraries

The binary requires `libSDL2` and `libSDL2_ttf`. These are not in the standard SSP rootfs and must be provided manually. The simplest approach is to copy them into the same directory as the binary:

```bash
scp libSDL2-2.0.so.0 libSDL2_ttf-2.0.so.0 root@192.168.0.150:/media/BOOT/er301/
```

you will need to set the `LD_LIBRARY_PATH`  to /media/BOOT/er301


All other runtime dependencies (`libm`, `libdl`, `libstdc++`, `libgcc_s`, `libc`, `libpthread`, `libfreetype`, `libpng16`, `libz`, `libbz2`) are part of the standard SSP rootfs.

Full `ldd` output on the target for reference:

```
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

### 4. Config file
configure to use the sdcard rather than ~/.od

/media/BOOT/er301/emu.config
```
XROOT /media/BOOT/er301/xroot
REAR_ROOT /media/BOOT/er301/rear
FRONT_ROOT /media/BOOT/er301/front
SESSION /media/BOOT/er301/emu.session
```

### 5. Running

```bash
LD_LIBRARY_PATH=/media/BOOT/er301 /media/BOOT/er301/er301.elf -c /media/BOOT/er301/emu.config
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
make core
```

or, if you keep only `SSP_BUILDROOT` exported:

```bash
export BUILDROOT=$SSP_BUILDROOT
make emu
make core
```

### Missing SDL2/SDL2_ttf/fftw during link

Confirm the target sysroot includes:

- `usr/include/SDL2`
- `usr/lib/libSDL2*`
- `usr/lib/libSDL2_ttf*`
- `usr/lib/libfftw3f*`

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
