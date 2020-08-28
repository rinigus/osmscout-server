#!/bin/bash
set -Eeuo pipefail

ROOT_DIR=$(git rev-parse --show-toplevel)
LIBOSMSCOUT_SRC_DIR="${ROOT_DIR}/libs/libosmscout"
LIBPOSTAL_SRC_DIR="${ROOT_DIR}/libs/libpostal"
VALHALLA_SRC_DIR="${ROOT_DIR}/libs/valhalla"
FONTS_SRC_DIR="${ROOT_DIR}/libs/fonts"
PROJ_SRC_DIR="${ROOT_DIR}/libs/proj"

# Remove old downloads
rm -rf "${LIBOSMSCOUT_SRC_DIR}" "${LIBPOSTAL_SRC_DIR}" "${VALHALLA_SRC_DIR}" "${FONTS_SRC_DIR}" "${PROJ_SRC_DIR}"

# Download sources
git clone --recursive --shallow-submodules --depth 1 https://github.com/rinigus/libosmscout.git "${LIBOSMSCOUT_SRC_DIR}"
git clone --recursive --shallow-submodules -b devel https://github.com/rinigus/pkg-libpostal.git "${LIBPOSTAL_SRC_DIR}"
git clone --recursive -b master https://github.com/rinigus/pkg-valhalla-lite.git "${VALHALLA_SRC_DIR}"
