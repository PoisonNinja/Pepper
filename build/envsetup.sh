#!/bin/bash

function realpath() {
    cd $1; pwd
}

function croot() {
    cd $ROOT
}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TOOLCHAIN=$(realpath "$DIR/../toolchain")

export ROOT=$(realpath "$DIR/..")
export PATH="$TOOLCHAIN/bin:$TOOLCHAIN/local/bin:$PATH"
export -f croot

alias cproto='cproto -e -I$ROOT/strawberry/include'

echo "ROOT = $ROOT"
echo "PATH = $PATH"
alias cproto

echo
echo "Environment ready"
