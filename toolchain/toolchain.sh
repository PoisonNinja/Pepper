#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PREFIX="$DIR/local"
PATH="$PREFIX/bin:$PATH"
SYSROOT=$(cd $DIR/../hdd; pwd)

TARGET_ARCH="${TARGET_ARCH:-x86_64}"
TARGET="$TARGET_ARCH-pepper"

GCCVER="gcc-7.3.0"
BINUTILSVER="binutils-2.30"
NEWLIBVER="newlib-3.0.0"
GCCURL="http://www.netgull.com/gcc/releases/$GCCVER/$GCCVER.tar.xz"
BINUTILSURL="http://ftp.gnu.org/gnu/binutils/$BINUTILSVER.tar.xz"
NEWLIBURL="ftp://sourceware.org/pub/newlib/$NEWLIBVER.tar.gz"

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

SKIP_DEPS=false
SKIP_GCC=false
SKIP_BINUTILS=false
SKIP_NEWLIB=false

LOCAL_NEWLIB=""

# Useful functions
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

function patchdir() {
    echo "Patching $1..."
    pushd $1 > /dev/null
    patch -p1 < $2
    popd > /dev/null
}

function bail() {
    error
    echo "An error occurred. Please check the output and file an issue on Github"
    exit 1
}

function realpath() {
    cd "$1" || bail; pwd
}

PACKAGES="build-essential clang-format cmake cproto curl genext2fs grub-common libmpfr-dev libmpc-dev libgmp3-dev libncurses5-dev nasm ninja-build qemu texinfo xorriso"

trap cleanup EXIT

while test $# -gt 0
do
    case "$1" in
        --skip-dependencies)
            SKIP_DEPS=true
            ;;
        --skip-gcc)
            SKIP_GCC=true
            ;;
        --skip-binutils)
            SKIP_BINUTILS=true
            ;;
        --skip-newlib)
            SKIP_NEWLIB=true
            ;;
        --local-newlib)
            LOCAL_NEWLIB="$2"
            shift
            ;;
    esac
    shift
done


echo "Target information:"
echo "Architecture:" "$TARGET_ARCH"
echo

if [[ $SKIP_DEPS == false ]]
then

    HOST_OS="Unknown"
    HOST_ARCH="Unknown"

    if [ -f /etc/lsb-release ]; then
        . /etc/lsb-release
        HOST_OS=$DISTRIB_ID
    elif [ -f /etc/debian_version ]; then
        HOST_OS=Debian
    elif [ -f /etc/redhat-release ]; then
        HOST_OS="Red Hat"
    else
        HOST_OS=$(uname -s)
    fi

    case $(uname -m) in
    x86_64)
        HOST_ARCH=x64
        ;;
    i*86)
        HOST_ARCH=x86
        ;;
    *)
        ;;
    esac

    echo "Host information:"
    echo "OS:" "$HOST_OS"
    echo "Architecture:" "$HOST_ARCH"

    input=""

    echo -n "Is the information correct? (y/n) "
    read -r input

    if [[ "$input" == "y" ]]
    then
        if [[ "$HOST_OS" == "Ubuntu" ]] || [[ "$HOST_OS" == "Debian" ]]
        then
            . /etc/lsb-release
            sudo apt update
            sudo apt -fy install build-essential clang-format cmake cproto curl genext2fs grub-common libmpfr-dev libmpc-dev libgmp3-dev libncurses5-dev nasm ninja-build qemu texinfo xorriso
            # EFI installations are missing BIOS boot files, so using
            # grub-mkrescue would make unbootable disks
            if [[ -d "/sys/firmware/efi" ]]
            then
                echo ""
                echo "EFI system detected. Installing grub-pc-bin..."
                echo ""
                sudo apt install grub-pc-bin
            # WSL is also missing BIOS boot files
            elif grep -q Microsoft /proc/version
            then
                echo ""
                echo "Windows Subsystem for Linux detected. Installing grub-pc-bin..."
                echo ""
                sudo apt install grub-pc-bin
            fi
        elif [[ "$HOST_OS" == "Darwin" ]]
        then
            if [ ! -f "$(which brew)" ]
            then
                /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
            fi
            brew install mpfr gmp libmpc nasm genext2fs qemu xorriso cproto cmake ninja
            brew install $DIR/automake@1.12.rb
            brew install grep --with-default-names
            brew install findutils --with-default-names
        else
            error
            echo "Your OS is currently not supported."
            echo
            echo "Please install the following packages: $PACKAGES"
            echo
            echo "Press enter when you have installed those packages to continue..."
            read -r
        fi
    else
        error
        echo "Please install the following packages: $PACKAGES"
        echo
        echo "Press enter when you have installed those packages to continue..."
        read -r
    fi
else
    echo "Skipping dependencies installation"
fi

pushd "$TMPDIR" > /dev/null
mkdir -p "$DIR/local"
mkdir -p "$SYSROOT/usr/include"

echo
echo "Installing tools to $DIR/local"
echo

if [[ $SKIP_BINUTILS == false ]]
then
    download $BINUTILSURL binutils.tar.xz || bail
    extract binutils.tar.xz || bail
    patchdir $BINUTILSVER $DIR/$BINUTILSVER.patch
    mkdir build-binutils
    pushd build-binutils > /dev/null
    ../$BINUTILSVER/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-nls --disable-werror || bail
    make || bail
    make install || bail
    popd > /dev/null
else
    echo "Skipping binutils build"
fi

if [[ $SKIP_GCC == false ]]
then
    download $GCCURL gcc.tar.xz || bail
    extract gcc.tar.xz || bail
    patchdir $GCCVER $DIR/$GCCVER.patch
    mkdir build-gcc
    pushd build-gcc > /dev/null
    ../$GCCVER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --with-sysroot="$SYSROOT" || bail
    make all-gcc || bail
    make install-gcc || bail
    popd > /dev/null
else
    echo "Skipping GCC build"
fi

export PATH="$PREFIX/bin:$PATH"

if [[ $SKIP_NEWLIB == false ]]
then
    if [[ ! -z "$LOCAL_NEWLIB" ]]
    then
        echo "Using local newlib: $LOCAL_NEWLIB"
        cp $LOCAL_NEWLIB ./newlib.tar.gz || bail
    else
        download $NEWLIBURL newlib.tar.gz || bail
    fi
    extract newlib.tar.gz || bail
    patchdir $NEWLIBVER $DIR/$NEWLIBVER.patch
    cp -r $DIR/newlib/pepper $NEWLIBVER/newlib/libc/sys/pepper
    cp $DIR/newlib/pepper/$TARGET_ARCH/* $NEWLIBVER/newlib/libc/sys/pepper
    cp $DIR/newlib/pepper/$TARGET_ARCH/sys/* $NEWLIBVER/newlib/libc/sys/pepper/sys/
    mkdir build-newlib
    pushd build-newlib > /dev/null
    ../$NEWLIBVER/configure --prefix="/usr" --target="$TARGET" || bail
    make all || bail
    make DESTDIR="$SYSROOT" install || bail
    # Work around a newlib bug
    cp -R $SYSROOT/usr/$TARGET/* $SYSROOT/usr/
    cp $DIR/newlib/pepper/sys/* $SYSROOT/usr/include/sys/
    rm -r $SYSROOT/usr/$TARGET
    popd > /dev/null
else
    echo "Skipping newlib build"
fi

if [[ $SKIP_GCC == false ]]
then
    pushd build-gcc > /dev/null
    make all-target-libgcc || bail
    make install-target-libgcc || bail
    make all-target-libstdc++-v3 || bail
    make install-target-libstdc++-v3 || bail
    popd > /dev/null
else
    echo "Skipping libstdc++v3 build"
fi

popd > /dev/null
echo
echo "The toolchain has been installed successfully"
echo "Don't forget to activate the toolchain!"
echo
echo ". build/envsetup.sh"
if [[ "$HOST_OS" == "Darwin" ]]
then
    echo
    echo "GRUB does not come preinstalled on MacOS, and it isn't available from Homebrew unfortunately"
    echo "However, we require grub-mkrescue to create the bootable ISO"
    echo
    echo "Since you are running MacOS, please also run grub.sh located in the same folder as toolchain.sh"
fi
