# CMake Build Setup for ER-301 Percussa

## Summary

The CMake build for the ER-301 Percussa target is **standalone** and does not rely on prebuilt libraries in `testing/*`. All dependencies (lua54, miniz, lodepng) are built locally from source in `libs/`. SWIG bindings are generated during the build process.

## Build Commands

### Darwin (native):
```bash
cmake -B build
cmake --build build
```

### SSP cross-compile:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../xcSSP.cmake -B build.ssp
cmake --build build.ssp
```

### XMX cross-compile:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../xcXMX.cmake -B build.xmx
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
- **fftw3f** - Required on Darwin (via homebrew)
- **Threads** - Standard pthread library

## Build Status

| Target | Status |
|--------|--------|
| Darwin native | Working |
| SSP cross-compile | Not tested |
| XMX cross-compile | Not tested |
| Linux native | Not tested |

### Verified working on Darwin:
- `od` static library
- `lua54` static library
- `miniz` static library
- `lodepng` static library
- `rtaudio` object library
- `percussa` executable

## Notes

- C++ standard: C++11
- C standard: C11 (gnu11 for platform compatibility)
- Toolchain files (`xcSSP.cmake`, `xcXMX.cmake`) for cross-compilation
- SWIG found via `find_package(SWIG)`
- All library sources compiled with `-fPIC`