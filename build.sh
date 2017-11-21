#!/bin/bash

git submodule update --init --recursive

cd lib/discord-rpc

mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=.
cmake --build . --config Release --target install
cd ../../..


mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make

cd ..

