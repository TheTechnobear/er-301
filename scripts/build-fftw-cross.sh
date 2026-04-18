#!/usr/bin/env bash
set -euo pipefail

# Cross-build FFTW from official tarballs for arm-linux-gnueabihf using LLVM,
# installing into a project-local staging tree by default.
#
# This does NOT modify your SSP sysroot unless you choose a DESTDIR inside it.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

FFTW_VERSION="${FFTW_VERSION:-3.3.10}"
FFTW_TARBALL="fftw-${FFTW_VERSION}.tar.gz"
FFTW_DIRNAME="fftw-${FFTW_VERSION}"
FFTW_URL="${FFTW_URL:-https://www.fftw.org/${FFTW_TARBALL}}"

TARGET="${TARGET:-arm-linux-gnueabihf}"
TOOLSROOT="${TOOLSROOT:-/opt/homebrew/opt/llvm/bin}"

# Keep install local to project by default, as requested.
DESTDIR="${DESTDIR:-$REPO_ROOT/.local/fftw-stage}"
PREFIX="${PREFIX:-/usr}"

# Optional sysroot for include/lib resolution during compile/link.
if [[ -n "${SSP_BUILDROOT:-}" ]]; then
  SYSROOT_DEFAULT="${SSP_BUILDROOT}/arm-rockchip-linux-gnueabihf/sysroot"
elif [[ -n "${BUILDROOT:-}" ]]; then
  SYSROOT_DEFAULT="${BUILDROOT}/arm-rockchip-linux-gnueabihf/sysroot"
else
  SYSROOT_DEFAULT=""
fi
SYSROOT="${SYSROOT:-$SYSROOT_DEFAULT}"
GCCROOT="${GCCROOT:-}"
if [[ -z "$GCCROOT" && -n "${SSP_BUILDROOT:-}" ]]; then
  GCCROOT="${SSP_BUILDROOT}/lib/gcc/arm-rockchip-linux-gnueabihf/8.4.0"
elif [[ -z "$GCCROOT" && -n "${BUILDROOT:-}" ]]; then
  GCCROOT="${BUILDROOT}/lib/gcc/arm-rockchip-linux-gnueabihf/8.4.0"
fi

BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/.build/fftw-cross}"
SRC_DIR="$BUILD_DIR/src"
PKG_DIR="$BUILD_DIR/pkg"
STAMP_DIR="$BUILD_DIR/stamps"

mkdir -p "$SRC_DIR" "$PKG_DIR" "$STAMP_DIR" "$DESTDIR"

require_tool() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "error: missing required tool: $1" >&2
    exit 1
  fi
}

require_tool curl
require_tool tar
require_tool make

if [[ ! -x "$TOOLSROOT/clang" ]]; then
  echo "error: clang not found at $TOOLSROOT/clang" >&2
  exit 1
fi
if [[ ! -x "$TOOLSROOT/clang++" ]]; then
  echo "error: clang++ not found at $TOOLSROOT/clang++" >&2
  exit 1
fi
if [[ ! -x "$TOOLSROOT/llvm-ar" ]]; then
  echo "error: llvm-ar not found at $TOOLSROOT/llvm-ar" >&2
  exit 1
fi
if [[ ! -x "$TOOLSROOT/llvm-ranlib" ]]; then
  echo "error: llvm-ranlib not found at $TOOLSROOT/llvm-ranlib" >&2
  exit 1
fi

if [[ -n "$SYSROOT" && ! -d "$SYSROOT" ]]; then
  echo "error: SYSROOT does not exist: $SYSROOT" >&2
  exit 1
fi
if [[ -z "$GCCROOT" || ! -d "$GCCROOT" ]]; then
  echo "error: GCCROOT not found. Set GCCROOT explicitly or provide SSP_BUILDROOT/BUILDROOT." >&2
  exit 1
fi

TARBALL_PATH="$PKG_DIR/$FFTW_TARBALL"
SRC_PATH="$SRC_DIR/$FFTW_DIRNAME"

if [[ ! -f "$TARBALL_PATH" ]]; then
  echo "==> Downloading $FFTW_URL"
  curl -L --fail --retry 3 -o "$TARBALL_PATH" "$FFTW_URL"
else
  echo "==> Reusing existing tarball: $TARBALL_PATH"
fi

if [[ ! -d "$SRC_PATH" ]]; then
  echo "==> Extracting $TARBALL_PATH"
  tar -xzf "$TARBALL_PATH" -C "$SRC_DIR"
else
  echo "==> Reusing extracted source: $SRC_PATH"
fi

CROSS_FLAGS=("--target=$TARGET")
if [[ -n "$SYSROOT" ]]; then
  CROSS_FLAGS+=("--sysroot=$SYSROOT")
fi

LINK_FLAGS=("-fuse-ld=lld")
if [[ -n "$SYSROOT" ]]; then
  LINK_FLAGS+=("-L$SYSROOT/lib" "-B$SYSROOT/lib" "-Wl,-rpath-link,$SYSROOT/lib")
  LINK_FLAGS+=("-L$SYSROOT/usr/lib" "-B$SYSROOT/usr/lib" "-Wl,-rpath-link,$SYSROOT/usr/lib")
fi
LINK_FLAGS+=("-L$GCCROOT" "-B$GCCROOT" "-Wl,-rpath-link,$GCCROOT")

export CC="$TOOLSROOT/clang ${CROSS_FLAGS[*]}"
export CXX="$TOOLSROOT/clang++ ${CROSS_FLAGS[*]}"
export AR="$TOOLSROOT/llvm-ar"
export RANLIB="$TOOLSROOT/llvm-ranlib"
export LDFLAGS="${LINK_FLAGS[*]}"

# pkg-config can help if FFTW configure probes need dependency lookups in sysroot.
if [[ -n "$SYSROOT" ]]; then
  export PKG_CONFIG_DIR=""
  export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig"
  export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
fi

pushd "$SRC_PATH" >/dev/null

CONFIG_STAMP="$STAMP_DIR/configured-${FFTW_VERSION}-${TARGET}"
if [[ ! -f "$CONFIG_STAMP" ]]; then
  echo "==> Configuring FFTW"
  ./configure \
    --host="$TARGET" \
    --prefix="$PREFIX" \
    --enable-single \
    --disable-fortran \
    --disable-shared \
    --enable-static
  touch "$CONFIG_STAMP"
else
  echo "==> Configure step already completed (remove $CONFIG_STAMP to reconfigure)"
fi

echo "==> Building FFTW"
make -j"${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"

echo "==> Installing into DESTDIR: $DESTDIR"
make DESTDIR="$DESTDIR" install

popd >/dev/null

echo
echo "FFTW cross-build complete."
echo "GCCROOT used: $GCCROOT"
echo "Staged files are under: $DESTDIR$PREFIX"
echo "Expected key outputs:"
echo "  $DESTDIR$PREFIX/include/fftw3.h"
echo "  $DESTDIR$PREFIX/lib/libfftw3f.a"
echo
echo "To force a clean reconfigure, remove: $STAMP_DIR"
