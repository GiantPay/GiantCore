#!/bin/bash

cd depends
make -j4 HOST=x86_64-pc-linux-gnu

cd ..
./autogen.sh
./configure --prefix=`pwd`/depends/x86_64-pc-linux-gnu
make -j4

