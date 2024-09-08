#!/bin/sh

set -e

OUTPUT=/custom_files/tiles_in_packages.json

if [ -f "$OUTPUT" ]; then
  echo "File already exists: $OUTPUT"
  echo "Skipping generation"
  exit 0
fi

echo "Generate Valhalla tiles2packs"
tiles2packs > $OUTPUT
