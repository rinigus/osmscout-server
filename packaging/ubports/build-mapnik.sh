#!/bin/bash
set -Eeuo pipefail

cp -r "${SRC_DIR}"/mapnik/* .

PROJ_LIB_INSTALL_DIR="${ROOT}/build/${ARCH_TRIPLET}/proj/install"
PROJ_INCLUDE_DIR="${PROJ_LIB_INSTALL_DIR}/include"
PROJ_SHARE_DIR="${PROJ_LIB_INSTALL_DIR}/share/proj"
PROJ_LIBS_DIR="${PROJ_LIB_INSTALL_DIR}/lib"

export LD_LIBRARY_PATH="$PROJ_LIBS_DIR"

./configure \
	DESTDIR="${INSTALL_DIR}" \
	PREFIX="/usr" \
	CXX="${ARCH_TRIPLET}-g++" \
	CC="${ARCH_TRIPLET}-gcc" \
	CUSTOM_CXXFLAGS="-fPIC -g0 -DMAPNIK_USE_PROJ4 -DHAVE_PNG" \
	CUSTOM_CFLAGS="-fPIC -g0" \
	PROJ_INCLUDES="${PROJ_INCLUDE_DIR}" \
	PROJ_LIBS="${PROJ_LIBS_DIR}" \
	INPUT_PLUGINS="sqlite,shape" \
	BENCHMARK=no \
	HOST="${ARCH_TRIPLET}" \
	LINKING=shared \
	OPTIMIZATION=2 \
	CPP_TESTS=no \
	CAIRO=no \
	PLUGIN_LINKING=static \
	MEMORY_MAPPED_FILE=no \
	DEMO=no \
	MAPNIK_INDEX=no \
	MAPNIK_RENDER=no

make reset
make
make install DESTDIR="${INSTALL_DIR}"
cp -r deps/mapbox/variant/include/mapbox "${INSTALL_DIR}/usr/include"
