#!/bin/bash

DIR="${0:A:h}"
TOOLCHAIN=$(realpath "$DIR/../toolchain")

export PATH="$TOOLCHAIN/local/bin:$PATH"

echo "PATH = $PATH"

echo
echo "Environment ready"
