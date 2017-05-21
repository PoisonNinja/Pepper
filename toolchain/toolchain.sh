#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

GCCVER="gcc-7.1.0"
BINUTILSVER="binutils-2.28"
GCCURL="http://www.netgull.com/gcc/releases/$GCCVER/$GCCVER.tar.bz2"
BINUTILSURL="http://ftp.gnu.org/gnu/binutils/$BINUTILSVER.tar.bz2"

PREFIX="$DIR/local"
TARGET=x86_64-elf
PATH="$PREFIX/bin:$PATH"

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

BUILD_BINUTILS=true
BUILD_GCC=true

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

function bail() {
    error
    echo "An error occurred. Please check the output and file an issue on Github"
    exit 1
}

function realpath() {
    cd "$1" || bail; pwd
}

PACKAGES="build-essential libmpfr-dev libmpc-dev libgmp3-dev nasm genext2fs texinfo qemu grub-common libncurses5-dev xorriso clang-format automake1.11 cproto"

trap cleanup EXIT

if [[ "$1" != "--skip" ]]
then

    OS="Unknown"
    ARCH="Unknown"

    if [ -f /etc/lsb-release ]; then
        . /etc/lsb-release
        OS=$DISTRIB_ID
    elif [ -f /etc/debian_version ]; then
        OS=Debian
    elif [ -f /etc/redhat-release ]; then
        OS="Red Hat"
    else
        OS=$(uname -s)
    fi

    case $(uname -m) in
    x86_64)
        ARCH=x64
        ;;
    i*86)
        ARCH=x86
        ;;
    *)
        ;;
    esac

    echo "Detected Information:"
    echo "OS:" "$OS"
    echo "Architecture:" "$ARCH"

    input=""

    echo -n "Is the information correct? (y/n) "
    read -r input

    if [[ "$input" == "y" ]]
    then
        if [[ "$OS" == "Ubuntu" ]] || [[ "$OS" == "Debian" ]]
        then
            . /etc/lsb-release
            sudo apt update
            sudo apt -fy install build-essential libmpfr-dev libmpc-dev libgmp3-dev nasm genext2fs texinfo qemu grub-common libncurses5-dev xorriso clang-format cproto
            # Only Ubuntu Precise ships with Automake 1.11
            if [[ "$DISTRIB_CODENAME" != "precise " ]]
            then
                echo ""
                echo "Your automake is too new! Installing an older version..."
                echo ""
                sudo apt-get -fy install automake1.11
                echo ""
                echo "Please select automake 1.11 when prompted..."
                echo ""
                sudo update-alternatives --config automake
            fi
            # EFI installations are missing BIOS boot files, so using
            # grub-mkrescue would make unbootable disks
            if [[ -d "/sys/firmware/efi" ]]
            then
                echo ""
                echo "EFI system detected. Installing grub-pc-bin..."
                echo ""
                sudo apt install grub-pc-bin
            fi
        elif [[ "$OS" == "Darwin" ]]
        then
            if [ ! -f "$(which brew)" ]
            then
                /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
            fi
            brew tap homebrew/versions
            brew install gcc mpfr gmp libmpc nasm genext2fs qemu xorriso clang-format cproto
            brew install grep --with-default-names
            brew install findutils --with-default-names
            brew install automake@1.12
            # We don't have the luxury of update-alternatives, so we have to
            # manually symlink the binaries
            if [ ! -f /usr/local/bin/aclocal ]
            then
                ln -s /usr/local/bin/aclocal112 /usr/local/bin/aclocal
            fi
            if [ ! -f /usr/local/bin/automake ]
            then
                ln -s /usr/local/bin/automake112 /usr/local/bin/automake
            fi
            # Update this as needed depending on what version ships with
            # Homebrew
            export CC=gcc-7
            export CXX=g++-7
            export CPP=cpp-7
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
fi
pushd "$TMPDIR" > /dev/null
mkdir -p "$DIR/local"

echo
echo "Installing tools to $DIR/local"
echo

if [[ $BUILD_BINUTILS == true ]]
then
    download $BINUTILSURL binutils.tar.bz2 || bail
    extract binutils.tar.bz2 || bail
    mkdir build-binutils
    pushd build-binutils > /dev/null
    ../$BINUTILSVER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror || bail
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
    ../$GCCVER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers || bail
    make all-gcc || bail
    make all-target-libgcc || bail
    make install-gcc || bail
    make install-target-libgcc || bail
    popd > /dev/null
fi

popd > /dev/null
echo
echo "The toolchain has been installed successfully"
echo "Don't forget to activate the toolchain!"
echo
echo ". build/envsetup.sh"
