#!/bin/bash
set -Eeuo pipefail

ROOT_DIR=$(git rev-parse --show-toplevel)
LIBPOSTAL_SRC_DIR="${ROOT_DIR}/libs/libpostal"
VALHALLA_SRC_DIR="${ROOT_DIR}/libs/valhalla"

# Remove old downloads
rm -rf "${LIBPOSTAL_SRC_DIR}" "${VALHALLA_SRC_DIR}"

# Download sources
git clone --recursive --shallow-submodules -b devel https://github.com/rinigus/pkg-libpostal.git "${LIBPOSTAL_SRC_DIR}"
git clone --recursive -b 3.0.9 https://github.com/rinigus/pkg-valhalla-lite.git "${VALHALLA_SRC_DIR}"
