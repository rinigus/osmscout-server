#!/bin/bash
set -Eeuo pipefail

ROOT_DIR=$(git rev-parse --show-toplevel)
CLONE_ARGS="--recursive --shallow-submodules --depth 1"
LIBPOSTAL_SRC_DIR="${ROOT_DIR}/libs/libpostal"
VALHALLA_SRC_DIR="${ROOT_DIR}/libs/valhalla"

# Remove old downloads
rm -rf "${LIBPOSTAL_SRC_DIR}" "${VALHALLA_SRC_DIR}"

# Download sources
git clone -b 1.0.0 ${CLONE_ARGS} https://github.com/rinigus/pkg-libpostal.git "${LIBPOSTAL_SRC_DIR}"
git clone -b 3.4.0 ${CLONE_ARGS} https://github.com/rinigus/pkg-valhalla-lite.git "${VALHALLA_SRC_DIR}"
