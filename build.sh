#! /usr/bin/bash

set -e

cmake -B build > /dev/null
make -j$(nproc --all) -C build > /dev/null
