#!/bin/bash
set -euxo pipefail
git clean -dxf
./autogen.sh
./configure --with-qrencode --with-zmq --with-gui --enable-lcov --enable-debug --enable-gprof --enable-extended-functional-tests --disable-hardening
make clean