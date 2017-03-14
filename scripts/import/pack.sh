#!/bin/bash

set -e

if [ $# -lt 2 ] ; then
	echo "Usage: $0 folder_to_pack version"
	exit 1
fi

TOPACK=$1
VERSION=$2

du -shb "$TOPACK" > "$TOPACK.size"
bzip2 "$TOPACK"/* || bzip2 "$TOPACK"/*/*
du -shb "$TOPACK" > "$TOPACK.size-compressed"
date +'%Y-%m-%d_%H:%M' > "$TOPACK.timestamp"
echo $VERSION > "$TOPACK.version"
