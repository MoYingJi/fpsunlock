#!/usr/bin/env sh

set -e

strip="x86_64-w64-mingw32-strip"

sh setup.sh --buildtype=release
meson compile -C build

$strip ./build/fpsunlock.exe
