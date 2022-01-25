#!/bin/bash

set -Eeuo pipefail

PROJECT_ROOT="$(git rev-parse --show-toplevel)"
CONFIG_FILE="${PROJECT_ROOT}/packaging/click/clickable.yaml"
BUILD_DIR="${PROJECT_ROOT}/build"
RELEASE_DIR="${PROJECT_ROOT}/click_release"

cd "$PROJECT_ROOT"
mkdir -p "${RELEASE_DIR}"

prepare() {
	clickable script prepare-deps -c ${CONFIG_FILE}
}

build() {
	local arch="$1"

	clickable build --all --clean --skip-review -c ${CONFIG_FILE} -a ${arch}
}

build_all() {
	local target_dir="${RELEASE_DIR}"

	build arm64
	build armhf
	build amd64

	mkdir -p "${target_dir}"
	cp ${BUILD_DIR}/*/app/osmscout-server*.click "${target_dir}"
}

prepare
build_all
