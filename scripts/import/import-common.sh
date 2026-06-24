#!/usr/bin/env bash

# Shared Podman import helpers. This file is sourced by import entrypoints.

# custom images
WGET_IMAGE="${WGET_IMAGE:-osmscout-wget}"
VALHALLA_TILES2PACKS_IMAGE="${VALHALLA_TILES2PACKS_IMAGE:-osmscout-valhalla-tiles2packs}"
GEOCODER_IMAGE="${GEOCODER_IMAGE:-osmscout-geocoder}"
POSTPROCESS_IMAGE="${POSTPROCESS_IMAGE:-osmscout-postprocess}"
NOMINATIM_GIS_IMAGE="${NOMINATIM_GIS_IMAGE:-osmscout-nominatim-gis}"
NOMINATIM_FEED_IMAGE="${NOMINATIM_FEED_IMAGE:-osmscout-nominatim-feed}"

# versions
VALHALLA_VERSION=3.4.0

# external images
OSMDOWNLOAD_IMAGE=openmaptiles/openmaptiles-tools
PLANETILER_IMAGE=ghcr.io/onthegomap/planetiler:0.10.2
POSTGRES_IMAGE=postgres:15-alpine
VALHALLA_IMAGE=ghcr.io/gis-ops/docker-valhalla/valhalla:${VALHALLA_VERSION}
HELPER_IMAGE=alpine

# helper functions
message() {
  printf '\n%s\n\n' "$*"
}

load_import_env() {
  local env_file="${1:-.env}"

  if [ -f "$env_file" ]; then
    set -a
    # shellcheck disable=SC1090
    source "$env_file"
    set +a
  fi
}

require_import_vars() {
  local var

  for var in "$@"; do
    if [ -z "${!var:-}" ]; then
      message "Environment variable ${var} is required"
      exit 1
    fi
  done
}

normalize_bool() {
  case "${1,,}" in
    true | 1 | yes | on)
      printf 'true'
      ;;
    *)
      printf 'false'
      ;;
  esac
}

wait_for_postgres() {
  local container="$1"
  message "Waiting for PostgreSQL..."

  until podman exec "$container" pg_isready -U postgres >/dev/null 2>&1; do
    sleep 2
  done
}

ensure_pod_absent() {
  if podman pod exists "$POD_NAME"; then
    message "Pod $POD_NAME already exists. Stop and remove it before starting a new import."
    exit 1
  fi
}

ensure_pod_exists() {
  if ! podman pod exists "$POD_NAME"; then
    message "Pod $POD_NAME does not exist. Create it before running with --skip-setup."
    exit 1
  fi
}

create_import_pod() {
  message "Creating Podman pod: $POD_NAME"
  podman pod create \
    --shm-size="${SHM_SIZE_DEFAULT}" \
    --name "$POD_NAME" \
    "$@"
}

wait_for_container_shutdown() {
  local container="$1"
  local timeout="$2"
  local elapsed=0

  message "Waiting up to ${timeout}s for ${container} container to stop cleanly..."

  while [ "$elapsed" -lt "$timeout" ]; do
    if ! podman container exists "$container"; then
      message "Nominatim container removed."
      return 0
    fi

    if [ "$(podman inspect -f '{{.State.Running}}' "$container")" != "true" ]; then
      message "Nominatim container stopped."
      return 0
    fi

    sleep 2
    elapsed=$((elapsed + 2))
  done

  message "Timed out waiting for ${container} container to stop; cleanup will remove the pod."
}
