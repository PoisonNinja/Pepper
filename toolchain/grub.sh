#!/bin/bash

TMPDIR=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')

pushd $TMPDIR > /dev/null
git clone git://git.savannah.gnu.org/grub.git
pushd grub > /dev/null
./autogen.sh
popd > /dev/null
mkdir build
pushd build > /dev/null
../grub/configure --disable-werror TARGET_CC=x86_64-elf-gcc TARGET_OBJCOPY=x86_64-elf-objcopy \
TARGET_STRIP=x86_64-elf-strip TARGET_NM=x86_64-elf-nm TARGET_RANLIB=x86_64-elf-ranlib --target=x86_64-elf
make
make install
popd > /dev/null
popd > /dev/null
