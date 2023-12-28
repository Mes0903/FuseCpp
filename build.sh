#!/bin/bash

mkdir mnt
mkdir build && cd build
cmake ..
cmake --build .