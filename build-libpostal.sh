#!/bin/bash
set -Eeuo pipefail

cp -r "${SRC_DIR}"/libpostal/* .

CFLAGS="-fPIC -lstdc++"
CXXFLAGS="-fPIC"
./bootstrap.sh

ARCH_FLAGS=""
if [[ ${ARCH_TRIPLET} == "arm-linux-gnueabihf" ]]; then
  ARCH_FLAGS="--with-cflags-scanner-extra=-marm"
fi

./configure "${ARCH_FLAGS}" --disable-sse2 --datadir="/usr/local/libpostal/data" --disable-data-download --enable-static --disable-shared --host="${ARCH_TRIPLET}"
make -j$(nproc)
make install DESTDIR="${INSTALL_DIR}"
