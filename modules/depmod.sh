#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

perl depmod.pl -b $DIR/../sysroot/lib/modules/ -k $DIR/../sysroot/boot/quark.kernel "$@"
