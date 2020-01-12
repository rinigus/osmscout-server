#!/bin/bash
set -Eeuo pipefail

LIBOSMSCOUT_SRC_DIR="${ROOT}/libs/libosmscout"
LIBPOSTAL_SRC_DIR="${ROOT}/libs/libpostal"
MAPNIK_SRC_DIR="${ROOT}/libs/mapnik"
VALHALLA_SRC_DIR="${ROOT}/libs/valhalla"

# Remove old downloads
rm -rf "${LIBOSMSCOUT_SRC_DIR}" "${LIBPOSTAL_SRC_DIR}" "${MAPNIK_SRC_DIR}" "${VALHALLA_SRC_DIR}"

# Download sources
git clone --recursive --shallow-submodules --depth 1 https://github.com/rinigus/libosmscout.git "${LIBOSMSCOUT_SRC_DIR}"
git clone --recursive --shallow-submodules -b devel https://github.com/rinigus/pkg-libpostal.git libs/libpostal "${LIBPOSTAL_SRC_DIR}"
git clone --recursive --shallow-submodules -b master https://github.com/rinigus/pkg-mapnik "${MAPNIK_SRC_DIR}"
git clone --recursive --shallow-submodules https://github.com/rinigus/pkg-valhalla-lite.git "${VALHALLA_SRC_DIR}"

# Patch Mapnik
cd "${MAPNIK_SRC_DIR}/mapnik"
patch -p1 < ../rpm/mapnik.twkb.patch
patch -p1 < ../rpm/mapnik.issue3384.patch

