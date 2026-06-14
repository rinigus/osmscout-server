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

: "${AREA:?AREA is required}"
: "${PBF:?PBF is required}"
: "${STORE_PLANET:?STORE_PLANET is required}"
: "${STORE_MBTILES:?STORE_MBTILES is required}"
: "${STORE_VALHALLA:?STORE_VALHALLA is required}"
: "${STORE_NOMINATIM:?STORE_NOMINATIM is required}"
: "${STORE_IMPORTED:?STORE_IMPORTED is required}"
: "${STORE_MISC:?STORE_MISC is required}"
: "${RAM_DEFALT_LIMIT:?RAM_DEFALT_LIMIT is required}"
: "${RAM_NOMINATIM_LIMIT:?RAM_NOMINATIM_LIMIT is required}"
: "${RAM_PLANETILER_LIMIT:?RAM_PLANETILER_LIMIT is required}"
: "${RAM_VALHALLA_LIMIT:?RAM_VALHALLA_LIMIT is required}"
: "${SHM_SIZE_DEFAULT:?SHM_SIZE_DEFAULT is required}"
: "${NOMINATIM_PASSWORD:?NOMINATIM_PASSWORD is required}"
: "${JAVA_TOOL_OPTIONS:?JAVA_TOOL_OPTIONS is required}"
: "${VALHALLA_VERSION:?VALHALLA_VERSION is required}"
: "${GEOCODER_JOBS:?GEOCODER_JOBS is required}"
: "${PLANETILER_IMAGE:?PLANETILER_IMAGE is required}"
: "${HELPER_IMAGE:?HELPER_IMAGE is required}"
: "${VALHALLA_IMAGE:?VALHALLA_IMAGE is required}"
: "${NOMINATIM_GIS_IMAGE:?NOMINATIM_GIS_IMAGE is required}"
: "${NOMINATIM_FEED_IMAGE:?NOMINATIM_FEED_IMAGE is required}"
: "${POSTGRES_IMAGE:?POSTGRES_IMAGE is required}"

PLANETILER_STORAGE_TMP="${PLANETILER_STORAGE_TMP:-}"
NOMINATIM_IMPORT_SEPARATE="${NOMINATIM_IMPORT_SEPARATE:-false}"
NOMINATIM_DATABASE_SERVER="${NOMINATIM_DATABASE_SERVER:-127.0.0.1:5432}"

POD_NAME="${POD_NAME:-osmscout-import}"
DB_CONTAINER="${DB_CONTAINER:-${POD_NAME}-nominatim}"
WGET_IMAGE="${WGET_IMAGE:-osmscout-wget}"
VALHALLA_TILES2PACKS_IMAGE="${VALHALLA_TILES2PACKS_IMAGE:-osmscout-valhalla-tiles2packs}"
POSTPROCESS_IMAGE="${POSTPROCESS_IMAGE:-osmscout-postprocess}"
NOMINATIM_SHUTDOWN_TIMEOUT="${NOMINATIM_SHUTDOWN_TIMEOUT:-300}"

case "${NOMINATIM_IMPORT_SEPARATE,,}" in
  true | 1 | yes | on)
    NOMINATIM_IMPORT_SEPARATE=true
    ;;
  *)
    NOMINATIM_IMPORT_SEPARATE=false
    ;;
esac

####################################
# helper functions
message() {
  printf '\n%s\n\n' "$*"
}

cleanup() {
  local status=$?

  if podman pod exists "$POD_NAME"; then
    echo
    echo "Remaining running containers in pod $POD_NAME:"
    podman ps --filter "pod=${POD_NAME}"

    echo
    echo "Removing Podman pod $POD_NAME..."
    podman pod rm -f "$POD_NAME"
  fi

  if [ "$status" -eq 0 ]; then
    message "Closing import"
  else
    message "ERROR: Closing import with error. Read the messages above for details."
  fi
  exit "$status"
}

trap cleanup EXIT

wait_for_postgres() {
  message "Waiting for PostgreSQL..."

  until podman exec "$DB_CONTAINER" pg_isready -U postgres >/dev/null 2>&1; do
    sleep 2
  done
}

wait_for_nominatim_shutdown() {
  local timeout="$1"
  local elapsed=0

  message "Waiting up to ${timeout}s for Nominatim container to stop cleanly..."

  while [ "$elapsed" -lt "$timeout" ]; do
    if ! podman container exists "$DB_CONTAINER"; then
      message "Nominatim container removed."
      return 0
    fi

    if [ "$(podman inspect -f '{{.State.Running}}' "$DB_CONTAINER")" != "true" ]; then
      message "Nominatim container stopped."
      return 0
    fi

    sleep 2
    elapsed=$((elapsed + 2))
  done

  message "Timed out waiting for Nominatim container to stop; cleanup will remove the pod."
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

####################################
# check if already started
if podman pod exists "$POD_NAME"; then
  message "Pod $POD_NAME already exists. Stop and remove it before starting a new import."
  exit 1
fi

####################################
# building images
message "Preparing helper images..."
build_image_if_missing "$WGET_IMAGE" \
  -f Dockerfile.wget \
  .

build_image_if_missing "$VALHALLA_TILES2PACKS_IMAGE" \
  --build-arg "VALHALLA_VERSION=${VALHALLA_VERSION}" \
  -f valhalla/Dockerfile.tiles2packs \
  valhalla

build_image_if_missing "$POSTPROCESS_IMAGE" \
  -f Dockerfile.postprocess \
  .

####################################
# init
message "Creating Podman pod: $POD_NAME"
podman pod create \
  --shm-size="${SHM_SIZE_DEFAULT}" \
  --name "$POD_NAME"

message "Setup required directories"
${SCRIPT_DIR}/prepare_docker.sh

####################################
# imports
message "Downloading area and Running Planetiler import..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-planetiler" \
  --memory="${RAM_PLANETILER_LIMIT}" \
  --entrypoint "" \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${STORE_MBTILES}:/data:z" \
  -v "${SCRIPT_DIR}/mapbox:/scripts_mapbox:z" \
  -e PLANETILER_STORAGE_TMP="${PLANETILER_STORAGE_TMP}" \
  -e JAVA_TOOL_OPTIONS="${JAVA_TOOL_OPTIONS}" \
  "$PLANETILER_IMAGE" \
  /scripts_mapbox/run_planetiler.sh /planet_pbf "${AREA}"

message "Preparing Valhalla import..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-valhalla-prepare" \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${STORE_VALHALLA}:/custom_files:z" \
  "$HELPER_IMAGE" \
  /bin/sh -c "rm /custom_files/*.pbf; ln -s /planet_pbf/${PBF} /custom_files"

message "Running Valhalla import..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-valhalla" \
  --memory="${RAM_VALHALLA_LIMIT}" \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${STORE_VALHALLA}:/custom_files:z" \
  -e serve_tiles=False \
  -e build_tar=False \
  -e build_admins=True \
  -e build_time_zones=True \
  -e build_transit=True \
  "$VALHALLA_IMAGE"

if [ "$NOMINATIM_IMPORT_SEPARATE" = true ]; then
  message "Skipping Nominatim auxiliary data download and import; using external database at ${NOMINATIM_DATABASE_SERVER}."
else
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

  message "Starting Nominatim database..."
  podman run -d \
    --pod "$POD_NAME" \
    --name "$DB_CONTAINER" \
    --memory="${RAM_NOMINATIM_LIMIT}" \
    -e POSTGRES_PASSWORD="${NOMINATIM_PASSWORD}" \
    -v "${STORE_NOMINATIM}:/var/lib/postgresql/data:Z" \
    "$NOMINATIM_GIS_IMAGE"

  wait_for_postgres

  message "Running Nominatim setup/import..."
  podman run --rm \
    --pod "$POD_NAME" \
    --name "${POD_NAME}-nominatim-setup" \
    -v "${STORE_PLANET}:/data:z" \
    -e PGHOST=127.0.0.1 \
    -e PGPASSWORD="${NOMINATIM_PASSWORD}" \
    -e OSM_FILENAME="${PBF}" \
    -e NOMINATIM_REPLICATION_URL="https://ftp5.gwdg.de/pub/misc/openstreetmap/planet.openstreetmap.org/replication/hour/" \
    -e NOMINATIM_REPLICATION_MAX_DIFF=3000 \
    -e NOMINATIM_REPLICATION_UPDATE_INTERVAL=86400 \
    "$NOMINATIM_FEED_IMAGE" \
    setup
fi

message "Preparing Valhalla packs for postprocessing..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-valhalla-packs" \
  -v "${STORE_VALHALLA}:/custom_files:z" \
  "$VALHALLA_TILES2PACKS_IMAGE"

message "Running postprocess import..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-postprocess" \
  --memory="${RAM_DEFALT_LIMIT}" \
  ${PODMAN_EXTRA_OPTIONS_POSTPROCESS:-} \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${STORE_MBTILES}:/mapbox-planet:z" \
  -v "${STORE_VALHALLA}:/valhalla:z" \
  -v "${STORE_IMPORTED}:/import:z" \
  -v "${STORE_MISC}:/osmscout:z" \
  -v "${SCRIPT_DIR}/hierarchy:/app/hierarchy:z" \
  -v "${SCRIPT_DIR}/provided:/app/provided:z" \
  -e GEOCODER_IMPORTER_POSTGRES="postgresql://postgres:${NOMINATIM_PASSWORD}@${NOMINATIM_DATABASE_SERVER}/nominatim" \
  -e GEOCODER_JOBS="${GEOCODER_JOBS}" \
  "$POSTPROCESS_IMAGE"

####################################
# cleanup
if [ "$NOMINATIM_IMPORT_SEPARATE" != true ]; then
  message "Shutting down Nominatim database..."
  podman run --rm \
    --pod "$POD_NAME" \
    --name "${POD_NAME}-postgres-shutdown" \
    -e PGHOST=127.0.0.1 \
    -e PGUSER=postgres \
    -e PGPASSWORD="${NOMINATIM_PASSWORD}" \
    "$POSTGRES_IMAGE" \
    sh -c "
      echo 'Waiting for few seconds...';
      sleep 1;
      echo 'Shutting down Nominatim database...';
      psql -c 'SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname = current_database() AND pid <> pg_backend_pid()';
      psql -c \"COPY (SELECT 1) TO PROGRAM 'pg_ctl stop -m smart --no-wait';\"
    "

  wait_for_nominatim_shutdown "$NOMINATIM_SHUTDOWN_TIMEOUT"
fi

message "OSM Scout import completed."
