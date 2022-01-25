#!/bin/bash
set -Eeuo pipefail

SCRIPT_SRC=osmscout-server.conf
SCRIPT_DST=/home/phablet/.config/upstart/

if [[ ! -f "${SCRIPT_DST}" ]]; then
	mkdir -p "${SCRIPT_DST}"
	cp "${SCRIPT_SRC}" "${SCRIPT_DST}"
	start osmscout-server || true
fi

osmscout-server-gui
