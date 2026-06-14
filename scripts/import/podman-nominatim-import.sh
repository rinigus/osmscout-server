#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"

if [ -f .env ]; then
  set -a
  # shellcheck disable=SC1091
  source .env
  set +a
fi

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/podman-import-common.sh"

: "${STORE_PLANET:?STORE_PLANET is required}"
: "${STORE_NOMINATIM_DB:?STORE_NOMINATIM_DB is required}"
: "${STORE_NOMINATIM_FLAT:?STORE_NOMINATIM_FLAT is required}"
: "${RAM_NOMINATIM_LIMIT:?RAM_NOMINATIM_LIMIT is required}"
: "${SHM_SIZE_DEFAULT:?SHM_SIZE_DEFAULT is required}"
: "${NOMINATIM_PASSWORD:?NOMINATIM_PASSWORD is required}"
: "${PBF:?PBF is required}"

POD_NAME="${POD_NAME:-nominatim-import}"
DB_CONTAINER="${POD_NAME}-nominatim"
POSTGRES_PORT="${POSTGRES_PORT:-15432}"

if podman pod exists "$POD_NAME"; then
  message "Pod $POD_NAME already exists. Stop and remove it before starting a new import."
  exit 1
fi

message "Preparing helper images..."
build_wget_image_if_missing
build_nominatim_images_if_missing

message "Downloading Nominatim auxiliary data..."
podman run --rm \
  --name "${POD_NAME}-wget" \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${SCRIPT_DIR}/scripts:/scripts:z" \
  "$WGET_IMAGE" \
  /scripts/get_urls.sh /planet_pbf \
    https://nominatim.org/data/wikimedia-importance.sql.gz \
    https://nominatim.org/data/gb_postcodes.csv.gz \
    https://nominatim.org/data/us_postcodes.csv.gz

message "Creating Podman pod..."
podman pod create \
  --shm-size="${SHM_SIZE_DEFAULT}" \
  --name "$POD_NAME" \
  -p "${POSTGRES_PORT}:5432"

message "Starting Nominatim database..."
podman run -d \
  --pod "$POD_NAME" \
  --name "$DB_CONTAINER" \
  --memory="${RAM_NOMINATIM_LIMIT}" \
  -e POSTGRES_PASSWORD="${NOMINATIM_PASSWORD}" \
  -v "${STORE_NOMINATIM_DB}:/var/lib/postgresql/data:Z" \
  "$NOMINATIM_GIS_IMAGE"

wait_for_postgres "$DB_CONTAINER"

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

message "Nominatim import completed."
message "Database container remains running in pod: $POD_NAME"
message "PostgreSQL is available on localhost:${POSTGRES_PORT}"
