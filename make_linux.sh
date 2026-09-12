#!/bin/bash
# make_linux.sh — build a self-contained OpenTyrian binary on Linux.
#
# SDL2 and SDL2_net are built from source and linked STATICALLY.  A distro's
# libSDL2 is linked against whatever that distro has (libdecor, Wayland,
# PulseAudio, PipeWire...), and bundling it drags every one of those in as a
# hard dependency on the user's machine.  SDL built from source instead
# loads all of those backends with dlopen at run time, so the resulting
# binary needs nothing beyond glibc and libm - and whatever the user has
# (X11 or Wayland, ALSA or Pulse) gets picked up when it runs.
#
# The freeware Tyrian 2.1 data is fetched into ./data if missing.
#
# Build requirements (headers only; nothing is linked from them):
#   Debian/Ubuntu:  sudo apt install build-essential cmake pkg-config curl unzip \
#       libasound2-dev libpulse-dev libx11-dev libxext-dev libxrandr-dev \
#       libxcursor-dev libxfixes-dev libxi-dev libxss-dev libwayland-dev \
#       libxkbcommon-dev libdecor-0-dev libegl1-mesa-dev libgl1-mesa-dev \
#       libdbus-1-dev libudev-dev
#
# Usage: ./make_linux.sh [data-dir]     (default: ./data, fetched if missing)
set -euo pipefail

cd "$(dirname "$0")"
DATA_ARG="${1:-data}"

have_data() { find "$1" -maxdepth 1 -iname "tyrian1.lvl" 2>/dev/null | grep -q .; }

if ! have_data "$DATA_ARG" && [ "$#" -ge 1 ]; then
    echo "ERROR: no Tyrian data in '$DATA_ARG'" >&2
    echo "Point make_linux.sh at your Tyrian 2.1 data, or run it with no argument" >&2
    echo "to download the freeware release into ./data automatically." >&2
    exit 1
fi
./get_data.sh "$DATA_ARG"
DATA_DIR="$(cd "$DATA_ARG" && pwd)"

# ---- static SDL2 + SDL2_net from source, installed under build/sdl ----
SDL2_VER="2.32.10"
SDL2_NET_VER="2.2.0"
PREFIX="$PWD/build/sdl"
NPROC="$(nproc 2>/dev/null || echo 4)"

# The license of anything we link in has to travel with the binary.  Keep the
# copies inside the prefix, since that is what CI caches: on a cache hit the
# source trees below are never unpacked.
install_license() {  # $1 = source directory, $2 = name in the package
    mkdir -p "$PREFIX/share/licenses"
    for name in LICENSE.txt LICENSE COPYING.txt COPYING; do
        if [ -f "$1/$name" ]; then
            cp "$1/$name" "$PREFIX/share/licenses/$2.txt"
            return 0
        fi
    done
    echo "ERROR: no license file found in $1" >&2
    exit 1
}

if [ ! -f "$PREFIX/lib/libSDL2.a" ]; then
    echo "Building SDL2 $SDL2_VER (static) ..."
    mkdir -p build/vendor
    curl -fL --progress-bar -o build/vendor/SDL2.tar.gz \
        "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VER/SDL2-$SDL2_VER.tar.gz"
    rm -rf "build/vendor/SDL2-$SDL2_VER"
    tar -xzf build/vendor/SDL2.tar.gz -C build/vendor
    cmake -S "build/vendor/SDL2-$SDL2_VER" -B "build/vendor/SDL2-$SDL2_VER/build" \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_STATIC_PIC=ON -DSDL_TEST=OFF >/dev/null
    cmake --build "build/vendor/SDL2-$SDL2_VER/build" -j"$NPROC" >/dev/null
    cmake --install "build/vendor/SDL2-$SDL2_VER/build" >/dev/null
    install_license "build/vendor/SDL2-$SDL2_VER" SDL2
fi

if [ ! -f "$PREFIX/lib/libSDL2_net.a" ]; then
    echo "Building SDL2_net $SDL2_NET_VER (static) ..."
    mkdir -p build/vendor
    curl -fL --progress-bar -o build/vendor/SDL2_net.tar.gz \
        "https://github.com/libsdl-org/SDL_net/releases/download/release-$SDL2_NET_VER/SDL2_net-$SDL2_NET_VER.tar.gz"
    rm -rf "build/vendor/SDL2_net-$SDL2_NET_VER"
    tar -xzf build/vendor/SDL2_net.tar.gz -C build/vendor
    cmake -S "build/vendor/SDL2_net-$SDL2_NET_VER" -B "build/vendor/SDL2_net-$SDL2_NET_VER/build" \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
        -DBUILD_SHARED_LIBS=OFF -DSDL2NET_SAMPLES=OFF >/dev/null
    cmake --build "build/vendor/SDL2_net-$SDL2_NET_VER/build" -j"$NPROC" >/dev/null
    cmake --install "build/vendor/SDL2_net-$SDL2_NET_VER/build" >/dev/null
    install_license "build/vendor/SDL2_net-$SDL2_NET_VER" SDL2_net
fi

# ---- the game, against the static SDL ----
# pkg-config resolves sdl2/SDL2_net from our prefix; --static pulls in the
# libraries SDL itself needs (m, dl, pthread, rt) for the static link.
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
make clean >/dev/null
make -j"$NPROC" \
    SDL_LDLIBS="$(pkg-config --static --libs-only-l sdl2 SDL2_net)"

echo
if ldd opentyrian | grep -q "libSDL2"; then
    echo "ERROR: opentyrian still links SDL2 dynamically" >&2
    exit 1
fi
echo "built: ./opentyrian (SDL2 linked statically)"
echo "run:   ./opentyrian --data=\"$DATA_DIR\""
