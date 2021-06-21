#!/bin/bash
make clean
./autogen.sh
./configure --with-incompatible-bdb --with-qrencode --with-zmq --with-gui --enable-lcov --enable-debug --enable-gprof
make clean