#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ARCH="x86_64"
FORCE=false
KERNEL_ARGS="-G Ninja"
USERSPACE_ARGS="-G Ninja"
NEWLIB=true

for i in "$@"
do
case $i in
    --architecture=*)
    ARCH="${i#*=}"
    shift # past argument=value
    ;;
    -f|--force)
    FORCE=true
    shift
    ;;
    --kernel_args=*)
    KERNEL_ARGS="${i#*=}"
    shift
    ;;
    --userspace_args=*)
    USERSPACE_ARGS="${i#*=}"
    shift
    ;;
    --no-newlib)
    NEWLIB=false
    shift
    ;;
    -h|--help)
    echo "    --architecture=       Set architecture"
    echo "-f, --force               Force overwrite existing build folders"
    echo "    --kernel_args=        Arguments for kernel CMake"
    echo "    --userspace_args=     Arguments for userspace CMake"
    echo "    --no-newlib           Skip newlib rebuild"
    exit 0
    ;;
    *)
    echo "Unknown option $i. Use -h/--help for usage"
    exit 1
    ;;
esac
done

case $ARCH in
    x86_64|i686)
    KERNEL_ARGS+=" -DARCH=$ARCH"
    USERSPACE_ARGS+=" -DARCH=$ARCH"
    ;;
    *)
    echo "Invalid architecture '$ARCH'"
    exit 1
    ;;
esac

echo "Architecture: $ARCH"
echo "Kernel CMake arguments: $KERNEL_ARGS"
echo "Userspace CMake arguments: $USERSPACE_ARGS"
echo ""

echo "Importing variables using envsetup.sh"
. $DIR/envsetup.sh > /dev/null

echo "Checking if requested compiler exists..."
command -v $ARCH-pepper-gcc > /dev/null 2>&1 || { echo >&2 "Compiler not found! Did you build the toolchain for the correct architecture?"; exit 1; }

if [[ $NEWLIB == true ]]
then
    echo "Rebuilding newlib..."
    TARGET_ARCH=$ARCH $DIR/../toolchain/newlib.sh
fi

echo "Setting up kernel..."
if (shopt -s nullglob dotglob; f=($DIR/../quark/build/*); ((${#f[@]})))
then
    if [[ $FORCE != true ]]
    then
        echo "Kernel build files exist already and -f/--force was not supplied. Aborting!"
        exit 1
    else
        rm -rf $DIR/../quark/build/ > /dev/null 2>&1
    fi
fi
mkdir $DIR/../quark/build
pushd $DIR/../quark/build > /dev/null
cmake .. $KERNEL_ARGS
popd > /dev/null

echo "Setting up userspace..."
if (shopt -s nullglob dotglob; f=($DIR/../userspace/build/*); ((${#f[@]})))
then
    if [[ $FORCE != true ]]
    then
        echo "Userspace build files exist already and -f/--force was not supplied. Aborting!"
        exit 1
    else
        rm -rf $DIR/../userspace/build/ > /dev/null 2>&1
    fi
fi
mkdir $DIR/../userspace/build
pushd $DIR/../userspace/build > /dev/null
cmake .. $USERSPACE_ARGS
popd > /dev/null

