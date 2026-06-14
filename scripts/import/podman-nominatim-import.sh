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
  message "Preparing helper images..."
  build_wget_image_if_missing
  build_nominatim_images_if_missing

  create_import_pod -p "${POSTGRES_PORT}:5432"
fi

run_nominatim_import

message "Nominatim import completed."
message "Database container remains running in pod: $POD_NAME"
