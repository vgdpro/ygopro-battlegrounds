#!/bin/bash
set -x
set -o errexit

TARGET_YGOPRO_BINARY_PATH=./ygopro-platforms/ygopro-platform-$TARGET_PATFORM
export EVENT_INCLUDE_DIR=$PWD/libevent-stable/include
export EVENT_LIB_DIR=$PWD/libevent-stable/lib
export IRRLICHT_INCLUDE_DIR=$PWD/irrlicht/include
export IRRLICHT_LIB_DIR=$PWD/irrlicht

git submodule update --init

./premake5 gmake --cc=clang --build-freetype --build-sqlite
cd build
make config=release -j4
cd ..

mkdir ygopro-platforms
mv bin/release/YGOPro.app $TARGET_YGOPRO_BINARY_PATH

install_name_tool -change /usr/local/lib/libirrklang.dylib @executable_path/../Frameworks/libirrklang.dylib $TARGET_YGOPRO_BINARY_PATH
strip $TARGET_YGOPRO_BINARY_PATH
