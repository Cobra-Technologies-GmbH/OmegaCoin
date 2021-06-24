#!/bin/bash
git clean -dxf
./autogen.sh
./configure --with-incompatible-bdb --with-qrencode --with-zmq --with-gui --enable-lcov --enable-debug --enable-gprof --enable-extended-functional-tests --disable-hardening
make clean