#!/bin/bash
# This file is for quick build and test
if [ ! -d "./build" ]; then
    mkdir build
fi
{
cd build
cmake ..
make
konsole -e ./p1
}