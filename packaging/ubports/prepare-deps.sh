#!/bin/bash
set -Eeuo pipefail

LIBOSMSCOUT_SRC_DIR="${ROOT}/libs/libosmscout"
LIBPOSTAL_SRC_DIR="${ROOT}/libs/libpostal"
VALHALLA_SRC_DIR="${ROOT}/libs/valhalla"

# Remove old downloads
rm -rf "${LIBOSMSCOUT_SRC_DIR}" "${LIBPOSTAL_SRC_DIR}" "${VALHALLA_SRC_DIR}"

# Download sources
git clone --recursive --shallow-submodules --depth 1 https://github.com/rinigus/libosmscout.git "${LIBOSMSCOUT_SRC_DIR}"
git clone --recursive --shallow-submodules -b devel https://github.com/rinigus/pkg-libpostal.git "${LIBPOSTAL_SRC_DIR}"
git clone --recursive -b master https://github.com/rinigus/pkg-valhalla-lite.git "${VALHALLA_SRC_DIR}"
