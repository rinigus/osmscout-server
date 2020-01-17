#!/bin/bash
set -Eeuo pipefail

cp -r "${SRC_DIR}"/mapnik/* .

make reset
./configure CCX=g++ INPUT_PLUGINS="sqlite,shape" PREFIX="/usr" CUSTOM_CXXFLAGS="-fPIC -g0" CUSTOM_CFLAGS="-fPIC -g0" LINKING=shared OPTIMIZATION=2 CPP_TESTS=no CAIRO=no PLUGIN_LINKING=static MEMORY_MAPPED_FILE=no DEMO=no MAPNIK_INDEX=no MAPNIK_RENDER=no --host="${ARCH_TRIPLET}" #ENABLE_STATS=True ENABLE_LOG=True
make -j$(nproc)
make install DESTDIR="${INSTALL_DIR}"

mkdir -p "${INSTALL_DIR}/usr/include"
cp -r deps/mapbox/variant/include/mapbox "${INSTALL_DIR}/usr/include"
