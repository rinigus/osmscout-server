#!/bin/bash
set -Eeuo pipefail

cp -r "${SRC_DIR}"/libpostal/* .

CFLAGS="-fPIC -lstdc++"
CXXFLAGS="-fPIC"
./bootstrap.sh

ARCH_FLAGS=""
case ${ARCH_TRIPLET} in
"arm-linux-gnueabihf" )
    ARCH_FLAGS="--with-cflags-scanner-extra=-marm"
    ;;
"aarch64-linux-gnu" )
    # TODO what do we need to pass here for arm64?
    ARCH_FLAGS="--with-cflags-scanner-extra=-marm"
    ;;
esac

./configure "${ARCH_FLAGS}" --disable-sse2 --datadir="/usr/local/libpostal/data" --disable-data-download --enable-static --disable-shared --host="${ARCH_TRIPLET}"
make -j$(nproc)
make install DESTDIR="${INSTALL_DIR}"
