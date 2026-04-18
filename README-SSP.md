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

- `BUILDROOT` (or `SSP_BUILDROOT`)
- `TOOLSROOT` (defaults to Homebrew LLVM on macOS arm64)
- `SYSROOT = $(BUILDROOT)/arm-rockchip-linux-gnueabihf/sysroot`
- `GCCROOT = $(BUILDROOT)/lib/gcc/arm-rockchip-linux-gnueabihf/8.4.0`
- `TRIPLE = arm-linux-gnueabihf`

## Prerequisites

1. SSP SDK/buildroot is installed and accessible.
2. LLVM toolchain is installed on host macOS.
   - Default expected path on Apple Silicon: `/opt/homebrew/opt/llvm/bin`
3. Target sysroot contains emulator dependencies (`SDL2`, `SDL2_ttf`, `fftw3f`).

## Environment

At minimum, set one of:

```bash
export SSP_BUILDROOT=/path/to/your/arm-rockchip-linux-gnueabihf_sdk-buildroot
# or
export BUILDROOT=/path/to/your/arm-rockchip-linux-gnueabihf_sdk-buildroot
```

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

This target builds:

1. `lua`
2. `miniz`
3. `lodepng`
4. `emu`

with:

- `ARCH=linux`
- `CROSS_COMPILE_EMU=1`

### Clean Cross Build Artifacts

```bash
make emu-cross-clean
```

## Alternative Invocation

If you prefer not to use the top-level shortcut target:

```bash
make ARCH=linux CROSS_COMPILE_EMU=1 -f scripts/lua.mk
make ARCH=linux CROSS_COMPILE_EMU=1 -f scripts/miniz.mk
make ARCH=linux CROSS_COMPILE_EMU=1 -f scripts/lodepng.mk
make ARCH=linux CROSS_COMPILE_EMU=1 -f scripts/emu.mk
```

## Output Layout

Cross emulator outputs are generated under:

- `testing/linux/emu/emu.elf` (for default `PROFILE=testing`)
- `testing/linux/libs/` (cross-built static libs)

For other profiles (`debug`, `release`), the first path segment changes accordingly.

## Notes on Build Behavior

When `CROSS_COMPILE_EMU=1`:

- Compiler switches to clang/clang++ cross invocation with `--target` and `--sysroot`.
- Linker uses `lld` with sysroot and gcc runtime search paths.
- `PKG_CONFIG_*` environment variables are set for sysroot-aware dependency resolution.
- Linux emulator CFLAGS switch from x86 (`-msse4`) to ARM (`-mcpu=cortex-a17 -mfloat-abi=hard -mfpu=neon-vfpv4`).
- Emulator include/library paths add `$(SYSROOT)/usr/include`, `$(SYSROOT)/usr/include/SDL2`, and `$(SYSROOT)/usr/lib`.

## Troubleshooting

### Error: `CROSS_COMPILE_EMU=1 requires BUILDROOT`

Set one of `SSP_BUILDROOT` or `BUILDROOT` before running `make emu-cross`.

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

### Known Issue (2026-04-18)

With:

- `SSP_BUILDROOT=/Users/kodiak/xc/ssp/arm-rockchip-linux-gnueabihf_sdk-buildroot`

the cross emulator build is currently blocked because the target sysroot does not contain all required emulator dependencies:

- missing `fftw3.h`
- missing `libfftw3f*`
- missing `libSDL2_ttf*`

Observed behavior:

- `lua`, `miniz`, and `lodepng` build successfully in cross mode.
- `emu` fails while compiling `emu/hal/fft.c` due to missing FFTW headers.
- if forced past compile, final link fails on unresolved SDL_ttf/FFTW symbols.

## Next Steps

1. Add target packages for `fftw3` and `sdl2_ttf` to the buildroot image/sysroot.
2. Rebuild or stage the target sysroot so it contains headers and libs:
   - `usr/include/fftw3.h`
   - `usr/lib/libfftw3f*`
   - `usr/lib/libSDL2_ttf*`
3. Re-run:

```bash
make emu-cross-clean
make emu-cross
```

4. Verify output exists at `testing/linux/emu/emu.elf`.

### LLVM tools not found

Set:

```bash
export TOOLSROOT=/opt/homebrew/opt/llvm/bin
```

or your local LLVM install path.
