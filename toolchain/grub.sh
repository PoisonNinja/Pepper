#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

TARGET=x86_64-pepper

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

PREFIX="$DIR/local"

BUILD_OBJCONV=true
BUILD_GRUB=true

function cleanup() {
    echo ""
    echo "Please wait, cleaning up..."
    rm -rf "$TMPDIR"
    # We only want to remove the symlinks if we are the ones who created it
    # It's possible that there is already an installation of automake
    # so we shouldn't remove it then
    if [ -f /usr/local/bin/.did_symlink_automake ]
    then
        rm /usr/local/bin/automake
        rm /usr/local/bin/aclocal
        rm /usr/local/bin/.did_symlink_automake
    fi
    rm -f $PREFIX/bin/objconv
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

echo
echo "Installing GRUB to $PREFIX"
echo

PATH="$PREFIX/bin:$PATH"

if [ ! -f /usr/local/bin/aclocal ]
then
    ln -s $(which aclocal112) /usr/local/bin/aclocal || bail
    ln -s $(which automake112) /usr/local/bin/automake || bail
    touch /usr/local/bin/.did_symlink_automake
fi

if [[ $BUILD_OBJCONV == true ]]
then
    git clone https://github.com/vertis/objconv.git
    pushd objconv > /dev/null
    g++ -o objconv -O2 src/*.cpp || bail
    cp objconv $PREFIX/bin/ || bail
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
    ../grub/configure --disable-werror TARGET_CC=$TARGET-gcc TARGET_OBJCOPY=$TARGET-objcopy \
    TARGET_STRIP=$TARGET-strip TARGET_NM=$TARGET-nm TARGET_RANLIB=$TARGET-ranlib --target=$TARGET \
    --prefix="$PREFIX" || bail
    make || bail
    make install || bail
    popd > /dev/null
fi

popd > /dev/null
