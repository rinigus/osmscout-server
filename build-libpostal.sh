#!/bin/bash
set -Eeuo pipefail

cp -r "${SRC_DIR}"/libpostal/* .

CFLAGS="-fPIC -lstdc++"
CXXFLAGS="-fPIC"
./bootstrap.sh
./configure --with-cflags-scanner-extra=-marm --disable-sse2 --datadir="${INSTALL_DIR}/usr/local/libpostal/data" --disable-data-download --enable-static --disable-shared --prefix="${INSTALL_DIR}" --exec-prefix="${INSTALL_DIR}" --host="${ARCH_TRIPLET}"
make -j$(nproc)
make install DESTDIR="${INSTALL_DIR}"
