#!/bin/bash
set -Eeuo pipefail

ROOT_DIR=$(git rev-parse --show-toplevel)
LIBOSMSCOUT_SRC_DIR="${ROOT_DIR}/libs/libosmscout"
LIBPOSTAL_SRC_DIR="${ROOT_DIR}/libs/libpostal"
VALHALLA_SRC_DIR="${ROOT_DIR}/libs/valhalla"
MAPNIK_SRC_DIR="${ROOT_DIR}/libs/mapnik"
FONTS_SRC_DIR="${ROOT_DIR}/libs/fonts"
PROJ_SRC_DIR="${ROOT_DIR}/libs/proj"

# Remove old downloads
rm -rf "${LIBOSMSCOUT_SRC_DIR}" "${LIBPOSTAL_SRC_DIR}" "${VALHALLA_SRC_DIR}" "${MAPNIK_SRC_DIR}" "${FONTS_SRC_DIR}" "${PROJ_SRC_DIR}"

# Download sources
git clone --recursive --shallow-submodules --depth 1 https://github.com/rinigus/libosmscout.git "${LIBOSMSCOUT_SRC_DIR}"
git clone --recursive --shallow-submodules -b devel https://github.com/rinigus/pkg-libpostal.git "${LIBPOSTAL_SRC_DIR}"
git clone --recursive -b master https://github.com/rinigus/pkg-valhalla-lite.git "${VALHALLA_SRC_DIR}"
git clone --recursive --shallow-submodules --depth 1 -b master https://github.com/rinigus/pkg-mapnik.git "${MAPNIK_SRC_DIR}"
git clone -b master --depth 1 https://github.com/rinigus/osmscout-server-fonts.git "${FONTS_SRC_DIR}"
wget -c http://download.osgeo.org/proj/proj-5.2.0.tar.gz -O - | tar -xz -C libs/ && mv "libs/proj-5.2.0" "${PROJ_SRC_DIR}"

# Apply patches
cd "${MAPNIK_SRC_DIR}/mapnik"
patch -p1 < ../rpm/mapnik.issue3384.patch
patch -p1 < ../rpm/mapnik.twkb.patch
