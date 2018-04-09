#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "Rebuilding newlib by calling toolchain.sh..."
echo

LOCAL_NEWLIB=$1

if [ ! -z $LOCAL_NEWLIB ]
then
    echo "Using local newlib at $1"
    echo
    $DIR/toolchain.sh --skip-dependencies --skip-binutils --skip-gcc --local-newlib $LOCAL_NEWLIB
else
    $DIR/toolchain.sh --skip-dependencies --skip-binutils --skip-gcc --local-newlib
fi
