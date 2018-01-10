#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

GCCVER="gcc-7.1.0"
BINUTILSVER="binutils-2.28"
GCCURL="http://www.netgull.com/gcc/releases/$GCCVER/$GCCVER.tar.bz2"
BINUTILSURL="http://ftp.gnu.org/gnu/binutils/$BINUTILSVER.tar.bz2"

TARGET=x86_64-elf

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

TOOLCHAIN_PREFIX="$TMPDIR/toolchain"
PREFIX="$DIR/local"

BUILD_BINUTILS=true
BUILD_GCC=true
BUILD_OBJCONV=true
BUILD_GRUB=true

function cleanup() {
    echo ""
    echo "Please wait, cleaning up..."
    rm -rf "$TMPDIR"
}

function error() {
    echo
    echo -e "\033[31mError:\033[0m"
}

function warning() {
    echo
    echo -e "\033[1;33mWarning:\033[0m"
}

function download() {
    curl -o "$2" "$1"
}

function extract() {
    echo "Extracting $1..."
    tar xf "$1"
}

function bail() {
    error
    echo "An error occurred. Please check the output and file an issue on Github"
    exit 1
}

trap cleanup EXIT

if [[ $(uname -s) != 'Darwin' ]]
then
    error
    echo "This script is intended for Mac devices only! Install grub from your package manager!"
    exit 1
fi

pushd $TMPDIR > /dev/null

mkdir -p "$TOOLCHAIN_PREFIX"

echo
echo "Installing temporary toolchain to $TOOLCHAIN_PREFIX"
echo "Installing GRUB to $PREFIX"
echo

brew install mpfr libmpc || bail

if [[ $BUILD_BINUTILS == true ]]
then
    download $BINUTILSURL binutils.tar.bz2 || bail
    extract binutils.tar.bz2 || bail
    mkdir build-binutils
    pushd build-binutils > /dev/null
    ../$BINUTILSVER/configure --target=$TARGET --prefix="$TOOLCHAIN_PREFIX" --disable-nls --disable-werror || bail
    make || bail
    make install || bail
    popd > /dev/null
fi

if [[ $BUILD_GCC == true ]]
then
    download $GCCURL gcc.tar.bz2 || bail
    extract gcc.tar.bz2 || bail
    mkdir build-gcc
    pushd build-gcc > /dev/null
    ../$GCCVER/configure --target=$TARGET --prefix="$TOOLCHAIN_PREFIX" --disable-nls --enable-languages=c,c++ --without-headers || bail
    make all-gcc || bail
    make all-target-libgcc || bail
    make install-gcc || bail
    make install-target-libgcc || bail
    popd > /dev/null
fi

PATH="$TOOLCHAIN_PREFIX/bin:$PATH"

if [[ $BUILD_OBJCONV == true ]]
then
    git clone https://github.com/vertis/objconv.git
    pushd objconv > /dev/null
    g++ -o objconv -O2 src/*.cpp || bail
    cp objconv /usr/local/bin/
    popd > /dev/null
fi

if [[ $BUILD_GRUB == true ]]
then
    git clone git://git.savannah.gnu.org/grub.git
    pushd grub > /dev/null
    ./autogen.sh
    popd > /dev/null
    mkdir build
    pushd build > /dev/null
    ../grub/configure --disable-werror TARGET_CC=x86_64-elf-gcc TARGET_OBJCOPY=x86_64-elf-objcopy \
    TARGET_STRIP=x86_64-elf-strip TARGET_NM=x86_64-elf-nm TARGET_RANLIB=x86_64-elf-ranlib --target=x86_64-elf \
    --prefix="$PREFIX" || bail
    make || bail
    make install || bail
    popd > /dev/null
fi

popd > /dev/null

rm /usr/local/bin/objconv || bail
brew uninstall mpfr libmpc || bail
