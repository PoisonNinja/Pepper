#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PREFIX="$DIR/local"
PATH="$PREFIX/bin:$PATH"

TARGET="x86_64-pc-none-elf"
TARGET_ARCH="x86_64"
TARGETS_TO_BUILD="X86"

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

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

    echo "Detected Information:"
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
            sudo apt -fy install build-essential nasm genext2fs texinfo qemu grub-common libncurses5-dev xorriso clang-format cproto cmake ninja-build
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
        elif [[ "$HOST_OS" == "Darwin" ]]
        then
            if [ ! -f "$(which brew)" ]
            then
                /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
            fi
            brew tap homebrew/versions
            brew install gmp nasm genext2fs qemu xorriso cproto cmake ninja
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

git clone https://github.com/llvm-mirror/llvm.git --depth=1
pushd llvm > /dev/null # $TMPDIR/llvm
pushd tools > /dev/null # $TMPDIR/llvm/tools
git clone https://github.com/llvm-mirror/clang.git --depth=1
git clone https://github.com/llvm-mirror/lld.git --depth=1
popd > /dev/null # $TMPDIR/llvm
mkdir build
pushd build > /dev/null # $TMPDIR/llvm/build
cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX="$PREFIX" -DLLVM_DEFAULT_TARGET_TRIPLE="$TARGET" -DLLVM_TARGET_ARCH="$TARGET_ARCH" -DLLVM_TARGETS_TO_BUILD="$TARGETS_TO_BUILD" -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --build . --target install
popd > /dev/null # TMPDIR/llvm
popd > /dev/null # $TMPDIR
popd > /dev/null
echo
echo "The toolchain has been installed successfully"
echo "Don't forget to activate the toolchain!"
echo
echo ". build/envsetup.sh"
