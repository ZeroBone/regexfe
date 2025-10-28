#!/usr/bin/env bash
set -eu

mkdir -p mimir
cd mimir

CUR=`pwd`

MIMIR_VERSION=393023f2b4b30d28ef027acf4a09f2980ac7cb71
MIMIR_REPO=https://github.com/AnyDSL/MimIR.git

if [ ! -e  "${CUR}/mimir" ]; then
    git clone ${MIMIR_REPO} mimir --depth=100
    cd mimir
    git reset --hard ${MIMIR_VERSION}
    git submodule update --init --recursive

    cd ${CUR}
fi

mkdir -p build
mkdir -p install
cd build
cmake ../mimir -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=${CUR}/install
make -j`nproc` install
