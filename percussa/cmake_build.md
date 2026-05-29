# CMake Build Setup for ER-301 Percussa

## Summary

The CMake build for the ER-301 Percussa target is **standalone** and does not rely on prebuilt libraries in `testing/*`. All dependencies (lua54, miniz, lodepng) are built locally from source in `libs/`. SWIG bindings are generated during the build process.

## Build Commands

### Using CMake Presets (Recommended)

CMake presets are defined in `CMakePresets.json` at the repo root:

```bash
# List available presets
cmake --list-presets

# Build mac (native Darwin)
cmake --preset mac
cmake --build --preset mac

# Build SSP (cross-compile for ARM 32-bit)
# First-time: build FFTW
DESTDIR=$PWD/.build/fftw-ssp PREFIX=/usr TOOLCHAIN_FLAVOR=ssp \
  SSP_BUILDROOT=/path/to/arm-rockchip-linux-gnueabihf_sdk-buildroot \
  TOOLSROOT=/opt/homebrew/opt/llvm/bin \
  bash scripts/build-fftw-cross.sh

cmake --preset ssp
cmake --build --preset ssp

# Build XMX (cross-compile for ARM 64-bit)
# First-time: build FFTW
DESTDIR=$PWD/.build/fftw-xmx PREFIX=/usr TOOLCHAIN_FLAVOR=xmx \
  XMX_BUILDROOT=/path/to/aarch64-rockchip-linux-gnu_sdk-buildroot \
  TOOLSROOT=/opt/homebrew/opt/llvm/bin \
  bash scripts/build-fftw-cross.sh

cmake --preset xmx
cmake --build --preset xmx
```

### VS Code + CMake Tools Extension

The [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) for VS Code provides a graphical interface for CMake with preset support.

**Setup:**
1. Install the CMake Tools extension
2. Open the project in VS Code
3. The extension will detect `CMakePresets.json` automatically

**Usage:**
- **Configure**: Press `F7` or use the CMake status bar to select a preset (mac/ssp/xmx)
- **Build**: Press `Ctrl+Shift+B` or click the Build button in the CMake Tools view
- The status bar shows the current preset and allows switching between presets

**First-time FFTW build for cross-compile targets:**
```bash
# For SSP
DESTDIR=$PWD/.build/fftw-ssp PREFIX=/usr TOOLCHAIN_FLAVOR=ssp \
  SSP_BUILDROOT=/path/to/arm-rockchip-linux-gnueabihf_sdk-buildroot \
  TOOLSROOT=/opt/homebrew/opt/llvm/bin \
  bash scripts/build-fftw-cross.sh

# For XMX
DESTDIR=$PWD/.build/fftw-xmx PREFIX=/usr TOOLCHAIN_FLAVOR=xmx \
  XMX_BUILDROOT=/path/to/aarch64-rockchip-linux-gnu_sdk-buildroot \
  TOOLSROOT=/opt/homebrew/opt/llvm/bin \
  bash scripts/build-fftw-cross.sh
```

### Manual Configuration

### Darwin (native):
```bash
cmake -B build
cmake --build build
```

### SSP cross-compile:
```bash
DESTDIR=$PWD/.build/fftw-ssp PREFIX=/usr TOOLCHAIN_FLAVOR=ssp \
  SSP_BUILDROOT=/path/to/arm-rockchip-linux-gnueabihf_sdk-buildroot \
  TOOLSROOT=/opt/homebrew/opt/llvm/bin \
  bash scripts/build-fftw-cross.sh

cmake -DCMAKE_TOOLCHAIN_FILE=../xcSSP.cmake \
  -DFFTW_STAGE_ROOT=$PWD/.build/fftw-ssp/usr \
  -B build.ssp
cmake --build build.ssp
```

### XMX cross-compile:
```bash
DESTDIR=$PWD/.build/fftw-xmx PREFIX=/usr TOOLCHAIN_FLAVOR=xmx \
  XMX_BUILDROOT=/path/to/aarch64-rockchip-linux-gnu_sdk-buildroot \
  TOOLSROOT=/opt/homebrew/opt/llvm/bin \
  bash scripts/build-fftw-cross.sh

cmake -DCMAKE_TOOLCHAIN_FILE=../xcXMX.cmake \
  -DFFTW_STAGE_ROOT=$PWD/.build/fftw-xmx/usr \
  -B build.xmx
cmake --build build.xmx
```

## CMake Files

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Root configuration, includes all sub-builds |
| `od/od.cmake` | Builds `od` static library (core DSP engine) |
| `percussa/percussa.cmake` | Builds `percussa` executable |
| `libs/lua54/lua54.cmake` | Builds lua54 from source |
| `libs/miniz/miniz.cmake` | Builds miniz from source |
| `libs/lodepng/lodepng.cmake` | Builds lodepng from source |

## Library Build Scripts

Each library has its own cmake file:

- **`libs/lua54/lua54.cmake`** - Compiles all lua54 sources except lua.c, luac.c (excluded per original makefile)
- **`libs/miniz/miniz.cmake`** - Compiles miniz.c
- **`libs/lodepng/lodepng.cmake`** - Compiles lodepng.cpp

## SWIG Generation

SWIG runs during build to generate `app_swig.cpp` from `od/glue/app.cpp.swig`:

```cmake
add_custom_command(
    OUTPUT ${SWIG_APP_SWG_CPP}
    COMMAND ${SWIG_EXECUTABLE} -lua -no-old-metatable-bindings -fvirtual -fcompact -c++ ${SWIG_INCLUDES} -o ${SWIG_APP_SWG_CPP} ${OD_SOURCE_ROOT}/glue/app.cpp.swig
    DEPENDS ${OD_SOURCE_ROOT}/glue/app.cpp.swig
)
```

The generated file is placed in the build directory and compiled as part of the percussa executable.

## Dependencies

### Built from source (local):
- **lua54** - Lua 5.4 interpreter (sources in `libs/lua54/`)
- **miniz** - ZIP compression (sources in `libs/miniz/`)
- **lodepng** - PNG loading (sources in `libs/lodepng/`)

### System (external):
- **SDL2** - Required on Darwin (via homebrew)
- **SDL2_ttf** - Required on Darwin (via homebrew)
- **fftw3f** - Required for cross-compile (built via `scripts/build-fftw-cross.sh`)
- **Threads** - Standard pthread library

## CMake Variables

### Configuration Options:
- `TARGET_SSP` / `TARGET_XMX` - Panel selection (default: SSP)
- `PERCUSSA_PANEL` - "ssp" or "xmx"
- `PERCUSSA_PLATFORM` - "host-sdl", "fbdev", or "plugin" (default: host-sdl on Darwin, fbdev on Linux)
- `FFTW_STAGE_ROOT` - Path to fftw3f staging directory (required for cross-compile)

## Build Status

| Target | Status |
|--------|--------|
| Darwin native | Working |
| SSP cross-compile | Working |
| XMX cross-compile | Working |
| Linux native | Not tested |

### Verified builds:
- `od` static library ✓
- `lua54` static library ✓
- `miniz` static library ✓
- `lodepng` static library ✓
- `rtaudio` object library ✓
- `percussa` executable (Darwin) ✓
- `percussa` executable (SSP cross-compile, ARM 32-bit) ✓
- `percussa` executable (XMX cross-compile, ARM 64-bit) ✓

## Notes

- C++ standard: C++11
- C standard: C11 (gnu11 for platform compatibility)
- Toolchain files (`xcSSP.cmake`, `xcXMX.cmake`) for cross-compilation
- SWIG found via `find_package(SWIG)`
- All library sources compiled with `-fPIC`
- FFTW must be built separately using `scripts/build-fftw-cross.sh` before cross-compiling
- FFTW staging directory should be outside the cmake build directory to avoid deletion on `rm -rf build*`

## VS Code Integration

For VS Code users, install the **CMake Tools** extension. It automatically recognizes `CMakePresets.json` and provides:
- Preset selection in the status bar
- Configure and Build commands via keyboard shortcuts
- Inline error highlighting and navigation

### Debugging

The project includes VS Code debug configurations in `.vscode/launch.json`:

| Configuration | Purpose | Notes |
|---------------|---------|-------|
| `Debug mac (cmake)` | Debug native macOS build | Uses lldb locally |
| `Debug SSP (cmake)` | Debug SSP cross-compile | Uses gdb for ARM remote debugging |
| `Debug XMX (cmake)` | Debug XMX cross-compile | Uses gdb for ARM64 remote debugging |
| `Debug EMU (makefile)` | Debug SDL2 emulator | Legacy makefile build |

Each debug configuration has a `preLaunchTask` that automatically rebuilds the target using CMake before launching.

**To debug:**
1. Select the debug configuration in VS Code's Debug panel
2. Press `F5` to start debugging
3. The target will be rebuilt automatically if needed

**Note:** Cross-compiled targets (SSP/XMX) require a remote debugger (gdbserver) running on the target hardware. Local debugging is only available for the mac native build.