#!/bin/bash
set -Eeuo pipefail

SCRIPT_SRC=osmscout-server.conf
SCRIPT_DST=/home/phablet/.config/upstart/osmscout-server.conf

if [[ ! -f "${SCRIPT_DST}" ]]; then
	cp "${SCRIPT_SRC}" "${SCRIPT_DST}"
	start osmscout-server
fi

osmscout-server-gui
