#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

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

if [[ $BUILD_OBJCONV == true ]]
then
    git clone https://github.com/vertis/objconv.git
    pushd objconv > /dev/null
    g++ -o objconv -O2 src/*.cpp
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
    ../grub/configure --disable-werror
    make
    make install
    popd > /dev/null
fi

popd > /dev/null
