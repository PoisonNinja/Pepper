#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "Rebuilding newlib by calling toolchain.sh..."

$DIR/toolchain.sh --skip-dependencies --skip-binutils --skip-gcc
