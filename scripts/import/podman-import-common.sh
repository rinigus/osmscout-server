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

nominatim_download_aux_data() {
  message "Downloading Nominatim auxiliary data..."
  podman run --rm \
    --pod "$POD_NAME" \
    --name "${POD_NAME}-wget" \
    -v "${STORE_PLANET}:/planet_pbf:z" \
    -v "${SCRIPT_DIR}/scripts:/scripts:z" \
    "$WGET_IMAGE" \
    /scripts/get_urls.sh /planet_pbf \
      https://nominatim.org/data/wikimedia-importance.sql.gz \
      https://nominatim.org/data/gb_postcodes.csv.gz \
      https://nominatim.org/data/us_postcodes.csv.gz
}

nominatim_start_database() {
  message "Starting Nominatim database..."
  podman run -d \
    --pod "$POD_NAME" \
    --name "$DB_CONTAINER" \
    --memory="${RAM_NOMINATIM_LIMIT}" \
    -e POSTGRES_PASSWORD="${NOMINATIM_PASSWORD}" \
    -v "${STORE_NOMINATIM_DB}:/var/lib/postgresql/data:Z" \
    "$NOMINATIM_GIS_IMAGE"

  wait_for_postgres "$DB_CONTAINER"
}

nominatim_run_setup() {
  message "Running Nominatim setup/import..."
  podman run --rm \
    --pod "$POD_NAME" \
    --name "${POD_NAME}-nominatim-setup" \
    -v "${STORE_PLANET}:/data:z" \
    -v "${STORE_NOMINATIM_FLAT}:/flatnode" \
    -e PGHOST=127.0.0.1 \
    -e PGPASSWORD="${NOMINATIM_PASSWORD}" \
    -e OSM_FILENAME="${PBF}" \
    -e NOMINATIM_FLATNODE_FILE=/flatnode/flat.node \
    "$NOMINATIM_FEED_IMAGE" \
    setup
}

run_nominatim_import() {
  nominatim_download_aux_data
  nominatim_start_database
  nominatim_run_setup
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
