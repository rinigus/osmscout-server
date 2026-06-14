#!/usr/bin/env bash

# Shared Podman import helpers. This file is sourced by import entrypoints.

# custom images
WGET_IMAGE="${WGET_IMAGE:-osmscout-wget}"
VALHALLA_TILES2PACKS_IMAGE="${VALHALLA_TILES2PACKS_IMAGE:-osmscout-valhalla-tiles2packs}"
POSTPROCESS_IMAGE="${POSTPROCESS_IMAGE:-osmscout-postprocess}"
NOMINATIM_GIS_IMAGE="${NOMINATIM_GIS_IMAGE:-osmscout-nominatim-gis}"
NOMINATIM_FEED_IMAGE="${NOMINATIM_FEED_IMAGE:-osmscout-nominatim-feed}"

message() {
  printf '\n%s\n\n' "$*"
}

build_image_if_missing() {
  local image="$1"
  shift

  if podman image exists "$image"; then
    message "Image $image already exists, skipping build."
    return
  fi

  message "Building image $image..."
  podman build -t "$image" "$@"
}

build_wget_image_if_missing() {
  build_image_if_missing "$WGET_IMAGE" \
    -f Dockerfile.wget \
    .
}

build_nominatim_images_if_missing() {
  build_image_if_missing "$NOMINATIM_GIS_IMAGE" \
    -f nominatim/gis/Dockerfile \
    nominatim/gis

  build_image_if_missing "$NOMINATIM_FEED_IMAGE" \
    -f nominatim/feed/Dockerfile \
    nominatim/feed
}

wait_for_postgres() {
  local container="$1"
  message "Waiting for PostgreSQL..."

  until podman exec "$container" pg_isready -U postgres >/dev/null 2>&1; do
    sleep 2
  done
}
