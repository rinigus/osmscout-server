#!/bin/bash
set -Eeuo pipefail

cp -r "${SRC_DIR}"/libpostal/* .

CFLAGS="-fPIC -lstdc++"
CXXFLAGS="-fPIC"
./bootstrap.sh

SCANNER_FLAGS=""
case ${ARCH_TRIPLET} in
"arm-linux-gnueabihf" )
    SCANNER_FLAGS="--with-cflags-scanner-extra=-marm"
    ;;
"aarch64-linux-gnu" )
    CFLAGS="${CFLAGS} -mmarch64"
    ;;
esac

./configure "${SCANNER_FLAGS}" --disable-sse2 --datadir="/usr/local/libpostal/data" --disable-data-download --enable-static --disable-shared --host="${ARCH_TRIPLET}"
make -j$(nproc)
make install DESTDIR="${INSTALL_DIR}"
