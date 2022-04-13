#!/bin/bash
set -x
set -o errexit

TARGET_YGOPRO_BINARY_PATH=./ygopro-platforms/ygopro-platform-$TARGET_PATFORM

git submodule update --init

./premake5 gmake --cc=clang --build-freetype --build-sqlite --event-include-dir=$PWD/libevent-stable/include --event-lib-dir=$PWD/libevent-stable/lib --irrlicht-include-dir=$PWD/irrlicht/include --irrlicht-lib-dir=$PWD/irrlicht
cd build
make config=release -j4
cd ..

mkdir ygopro-platforms
mv bin/release/YGOPro.app $TARGET_YGOPRO_BINARY_PATH

install_name_tool -change /usr/local/lib/libirrklang.dylib @executable_path/../Frameworks/libirrklang.dylib $TARGET_YGOPRO_BINARY_PATH
strip $TARGET_YGOPRO_BINARY_PATH
