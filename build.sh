#! /usr/bin/bash

set -o

cmake -B build
make -j$(nproc --all) -C build
