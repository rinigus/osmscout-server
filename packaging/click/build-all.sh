#!/bin/bash

set -Eeuo pipefail

PROJECT_ROOT="$(git rev-parse --show-toplevel)"
BUILD_DIR=build
RELEASE_DIR=click_release

cd "$PROJECT_ROOT"
mkdir -p "${RELEASE_DIR}"

prepare() {
	local config="packaging/click/clickable.json"

	clickable -c ${config} prepare-deps
}

build() {
	local arch="$1"
	local config="packaging/click/clickable.json"

	clickable -c ${config} -a ${arch} build-libs
	clickable -c ${config} -a ${arch} clean-build
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
