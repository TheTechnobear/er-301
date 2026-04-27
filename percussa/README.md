# ER-301 on Percussa SSP / XMX

This is a port of the [ER-301 Sound Computer](https://www.orthogonaldevices.com/er-301) to Percussa SSP and XMX hardware. It runs the full ER-301 application (DSP engine, Lua scripting, patch system) on Linux-based Percussa hardware using the same `xroot` Lua scripts and mod packages as the original firmware.

---

## End-User Guide

### Hardware Comparison

Understanding where SSP/XMX maps cleanly onto the ER-301 and where it diverges is essential for getting the most out of this port.

#### I/O Specs

| Feature | ER-301 | SSP | XMX |
|---------|--------|-----|-----|
| Audio outputs | 4 × OUT1–4, ±10V AC, 24-bit | 4 × OUT1–4, ±10V, 32-bit | 2 × OUT1–2, ±10V, 32-bit |
| Audio inputs | 4 × IN1–4, ±10V, 60 kHz, 16-bit DC | 4 × IN1–4, ±10V, 32-bit | 4 × IN1–4, ±10V, 32-bit |
| CV inputs | 12 × A1–D3, ±10V, 60 kHz, 16-bit DC | 12 × A1–D3, ±5V scaled to ±10V, 32-bit | 4 × A1–D1, ±5V scaled to ±10V, 32-bit |
| Gate inputs | 4 × G1–G4 | **Not available** | **Not available** |
| Sample rate | 48 or 96 kHz (firmware.cfg) | 48 kHz fixed | 48 kHz fixed |
| Coupling | Audio: AC; CV/gate: DC | All DC coupled | All DC coupled |
| Headphone | None | None | Mirror of OUT1–2 (hardware fixed) |

#### CV Voltage Scaling

The SSP and XMX use ±5V CV signals. The port scales these to the ER-301's expected ±10V range automatically. V/oct tracking is preserved. The scaling is applied in the HAL before samples reach the ER-301 DSP engine.

#### Missing I/O

**SSP:** Gate inputs G1–G4 are not present. Patches that use gate inputs will have those inlets unconnected (equivalent to a constant zero signal).

**XMX:** Gate inputs G1–G4 are not present. CV inputs are limited to A1–D1 (4 channels; A2–D3 are unavailable). Audio outputs are limited to OUT1–2 (OUT3–4 are unavailable).

#### Processing

| Feature | ER-301 | SSP / XMX |
|---------|--------|-----------|
| CPU | 1 GHz ARM Cortex-A8, single core | Quad-core ARM Cortex-A17 (RK3188) |
| DSP core | Dedicated core (hardware timer ISR) | Core 1 dedicated to audio callback |
| UI core | Same core as DSP (time-sliced) | Core 0 dedicated to OS and UI |
| RAM | 512 MB (~480 MB sample pool) | 1 GB |

Because the SSP/XMX runs the audio callback on a dedicated core isolated from the OS, real-time performance is generally comparable to the ER-301 hardware despite the different architecture.

---

### Controls

Both SSP and XMX use four encoders. Their mapping to ER-301 controls:

| Encoder | ER-301 function |
|---------|----------------|
| Encoder 1 | Data wheel (main navigation and value editing) |
| Encoders 2–4 | Emulate toggle/mode controls |
| Push encoder 4 | Links the next output with the current (output chaining) |

**SSP displays:** The ER-301's two displays (main 256×64, sub 128×64) are scaled up 3× to fit the SSP's larger screen. All other UI elements render at native resolution.

**XMX displays:** Both displays render at native resolution — no scaling is applied.

---

### Storage

| Storage | SSP | XMX |
|---------|-----|-----|
| Rear card (patches, mods) | Internal SD card | Internal SD card |
| Front card (samples) | USB mass storage when connected; falls back to internal SD | Planned (currently unsupported — OS limitation) |
| USB mass storage | Supported | Not yet supported |
| USB audio | Not yet supported | Not yet supported |

USB audio, if implemented, would expose all I/O channels including those currently missing (gates, extra CV channels, extra outputs).

---

### Menu Items

Some ER-301 menu items relate to hardware that does not exist on SSP/XMX (e.g., firmware update, hardware calibration). These items may appear in menus but have no effect or are hidden depending on the build.

---

## Building

### Prerequisites

All builds:
- LLVM toolchain (clang, lld)
- FFTW3 (built from source for cross-targets; see below)

macOS desktop builds additionally require:
- SDL2 and SDL2_ttf (install via Homebrew)

Hardware cross-builds additionally require:
- SSP SDK/buildroot (`arm-rockchip-linux-gnueabihf`)
- SDL2 and SDL2_ttf present in the sysroot

```bash
# macOS prerequisites
brew install llvm lld sdl2 sdl2_ttf
```

---

### macOS Desktop Build

The desktop build runs the SSP or XMX UI in an SDL2 window on your Mac. Useful for development and patch editing without hardware.

```bash
# Build for SSP (default on macOS)
make percussa

# Build for XMX
make percussa PERCUSSA_PANEL=xmx

# Clean
make percussa-clean
```

Output: `testing/macos/percussa/percussa` (or similar profile path)

Config is read from `~/.ssp/ssp.config` on first run; the file is created automatically if absent.

---

### SSP Hardware Build

```bash
# 1. Set your buildroot path
export BUILDROOT=/path/to/arm-rockchip-linux-gnueabihf_sdk-buildroot

# 2. Build FFTW for SSP (one-time setup)
TOOLCHAIN_FLAVOR=ssp DESTDIR=$PWD/testing/linux/fftw3 ./scripts/build-fftw-cross.sh

# 3. Build
make percussa TOOLCHAIN_FILE=scripts/toolchains/ssp.mk

# 4. Deploy to SSP (adjust IP as needed)
scp -O testing/linux/percussa/percussa.elf root@192.168.0.150:/media/BOOT/er301/er301.elf
scp -O -r xroot root@192.168.0.150:/media/BOOT/er301/
```

Create `/media/BOOT/er301/ssp.config` on the device:
```
XROOT /media/BOOT/er301/xroot
REAR_ROOT /media/BOOT/er301/rear
FRONT_ROOT /media/BOOT/er301/front
SESSION /media/BOOT/er301/ssp.session
```

Run on device:
```bash
LD_LIBRARY_PATH=/media/BOOT/er301 /media/BOOT/er301/er301.elf -c /media/BOOT/er301/ssp.config
```

---

### XMX Hardware Build

```bash
# 1. Set your buildroot path
export BUILDROOT=/path/to/xmx-buildroot

# 2. Build FFTW for XMX (one-time setup)
TOOLCHAIN_FLAVOR=xmx DESTDIR=$PWD/testing/linux/fftw3-xmx ./scripts/build-fftw-cross.sh

# 3. Build
make percussa TOOLCHAIN_FILE=scripts/toolchains/xmx.mk PERCUSSA_PANEL=xmx
```

Deployment follows the same pattern as SSP.

---

### Core Mod Package

The `core` package provides the built-in unit library. Build and deploy it alongside the binary:

```bash
make core                          # macOS (native)
make core TOOLCHAIN_FILE=scripts/toolchains/ssp.mk   # SSP cross-build

# Deploy to rear card for auto-install on next boot
scp -O testing/linux/mods-arm/core-*.pkg root@192.168.0.150:/media/BOOT/er301/rear/
```

---

### Advanced Build Options

#### Build without FFTW (disables convolution units)
```bash
make percussa WITH_FFTW=0
```

#### Override FFTW staging path
```bash
make percussa FFTW_STAGE_ROOT=/custom/path/usr
```

#### Panel and platform selection
```bash
# Explicit platform selection
make percussa PERCUSSA_PANEL=ssp PERCUSSA_PLATFORM=host-sdl
make percussa PERCUSSA_PANEL=xmx PERCUSSA_PLATFORM=fbdev

# Defaults:
#   macOS          → PERCUSSA_PANEL=ssp,  PERCUSSA_PLATFORM=host-sdl
#   Linux aarch64  → PERCUSSA_PANEL=xmx,  PERCUSSA_PLATFORM=fbdev
#   Other Linux    → PERCUSSA_PANEL=ssp,  PERCUSSA_PLATFORM=fbdev
```

#### Output layout
Build artifacts land in `testing/<profile>/<arch>/` by default. Override the architecture suffix:
```bash
make percussa BUILD_OUTPUT_SUFFIX=my-suffix
```

---

### Git Tags and Version Strings

Mod package filenames and firmware archive names are derived from Git tags. If tags are missing (common in forks), package names will be empty.

Fetch upstream tags:
```bash
git remote add upstream https://github.com/odevices/er-301
git fetch upstream --tags
```

Verify:
```bash
git describe --match "v*.*.*-*" --tags --abbrev=0
```

---

### Runtime Libraries (SSP Hardware Reference)

FFTW links statically; these shared libraries must be present on the target device:

| Library | Notes |
|---------|-------|
| `libSDL2-2.0.so.0` | Ship alongside binary |
| `libSDL2_ttf-2.0.so.0` | Ship alongside binary |
| `libm`, `libdl`, `libstdc++`, `libgcc_s`, `libc`, `libpthread` | Standard SSP rootfs |
| `libfreetype`, `libpng16`, `libz`, `libbz2` | Standard SSP rootfs |

---

### Font Configuration

The application searches for a TTF font at startup in this order:

1. `/usr/share/fonts/truetype/freefont/FreeSans.ttf`
2. `libs/SDL_FontCache/test/fonts/FreeSans.ttf`
3. `/usr/share/fonts/liberation/LiberationSans-Regular.ttf`
4. `/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf`

`INFO: Unable to open file...` messages in the log for earlier entries are harmless. On SSP hardware, Liberation Sans at path 3 is expected. Install the `liberation-fonts` package if it is absent.

---

## See Also

- `er301-architecture.md` — DSP engine internals, HAL structure, and how the Percussa port connects to the ER-301 firmware layer
- `er301-architecture.md` in project root — broad codebase overview and build system reference
- `CLAUDE.md` (project root) — codebase navigation guide for developers
