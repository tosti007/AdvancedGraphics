#! /usr/bin/bash
cmake -B build
make -j$(nproc --all) -C build
