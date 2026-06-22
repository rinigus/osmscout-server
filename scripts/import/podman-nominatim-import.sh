#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/podman-import-common.sh"

SUBTASK=false

while [ "$#" -gt 0 ]; do
  case "$1" in
    --subtask)
      SUBTASK=true
      ;;
    *)
      message "Unknown option: $1"
      exit 1
      ;;
  esac
  shift
done

if [ "$SUBTASK" != true ]; then
  load_import_env .env
fi

POD_NAME="${POD_NAME:-nominatim-import}"
DB_CONTAINER="${DB_CONTAINER:-${POD_NAME}-nominatim}"
POSTGRES_PORT="${POSTGRES_PORT:-15432}"
NOMINATIM_IMPORT_FLATNODE="$(normalize_bool "${NOMINATIM_IMPORT_FLATNODE:-true}")"

required_vars=(
  STORE_PLANET
  STORE_NOMINATIM_DB
  STORE_NOMINATIM_FLAT
  RAM_NOMINATIM_LIMIT
  SHM_SIZE_DEFAULT
  NOMINATIM_PASSWORD
  PBF
)

require_import_vars "${required_vars[@]}"

if [ "$SUBTASK" = true ]; then
  ensure_pod_exists
else
  ensure_pod_absent
  create_import_pod -p "${POSTGRES_PORT}:5432"
fi

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

message "Freezing Nominatim database..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-nominatim-freeze" \
  -e PGHOST=127.0.0.1 \
  -e PGPASSWORD="${NOMINATIM_PASSWORD}" \
  "$NOMINATIM_FEED_IMAGE" \
  nominatim freeze

if [ -f "${STORE_NOMINATIM_FLAT}/flat.node" ]; then
  message "Dropping flatnode left after import"
  rm "${STORE_NOMINATIM_FLAT}/flat.node"
fi

message "Nominatim import completed."
message "Database container remains running in pod: $POD_NAME"
