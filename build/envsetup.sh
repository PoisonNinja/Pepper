#!/bin/bash

# When cd is aliased or is a function (such as when RVM is installed),
# cd doesn't behave properly. To bypass this by forcing Bash to use the
# builtin, use command. Prefixing with a backslash is insufficient since
# backslashes only ignore aliases, and unalias cd won't work because it is
# a function not an alias

function realpath() {
    command cd $1; pwd
}

function croot() {
    command cd $ROOT
}

DIR="$( command cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TOOLCHAIN=$(realpath "$DIR/../toolchain")

export ROOT=$(realpath "$DIR/..")
export PATH="$TOOLCHAIN/bin:$TOOLCHAIN/local/bin:$PATH"
export -f croot

alias cproto='cproto -e -I$ROOT/mint/include'

echo "ROOT = $ROOT"
echo "PATH = $PATH"
alias cproto

echo
echo "Environment ready"
