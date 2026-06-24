#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/import-common.sh"

load_import_env .env

require_import_vars \
  AREA \
  PBF \
  STORE_PLANET \
  STORE_MBTILES \
  STORE_VALHALLA \
  STORE_NOMINATIM \
  STORE_IMPORTED \
  STORE_MISC \
  RAM_DEFALT_LIMIT \
  RAM_NOMINATIM_LIMIT \
  RAM_PLANETILER_LIMIT \
  RAM_VALHALLA_LIMIT \
  SHM_SIZE_DEFAULT \
  NOMINATIM_PASSWORD \
  JAVA_TOOL_OPTIONS \
  VALHALLA_VERSION \
  PLANETILER_IMAGE \
  HELPER_IMAGE \
  VALHALLA_IMAGE \
  POSTGRES_IMAGE

PLANETILER_STORAGE_TMP="${PLANETILER_STORAGE_TMP:-}"
NOMINATIM_IMPORT_SEPARATE="${NOMINATIM_IMPORT_SEPARATE:-false}"
NOMINATIM_DATABASE_SERVER="${NOMINATIM_DATABASE_SERVER:-127.0.0.1:5432}"

POD_NAME="${POD_NAME:-osmscout-import}"
DB_CONTAINER="${POD_NAME}-nominatim"
NOMINATIM_SHUTDOWN_TIMEOUT="${NOMINATIM_SHUTDOWN_TIMEOUT:-300}"

NOMINATIM_IMPORT_SEPARATE="$(normalize_bool "$NOMINATIM_IMPORT_SEPARATE")"

####################################
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
    message "Success: Closing import"
  else
    message "ERROR: Closing import with error. Read the messages above for details."
  fi
  exit "$status"
}

trap cleanup EXIT

####################################
# check if already started
ensure_pod_absent

####################################
# init
create_import_pod

####################################
# get data
"${SCRIPT_DIR}/podman-get-data.sh"

####################################
# imports
message "Downloading some specific datasets and Running Planetiler import..."
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
  /scripts_mapbox/run_planetiler.sh \
  --osm-path="/planet_pbf/${PBF}" \
  --download \
  --download_dir=/planet_pbf

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
    POD_NAME="$POD_NAME" \
    DB_CONTAINER="$DB_CONTAINER" \
    bash "${SCRIPT_DIR}/import-nominatim.sh" --subtask
fi

message "Running GeocoderNLP import..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-geocoder" \
  --memory="${RAM_DEFALT_LIMIT}" \
  ${PODMAN_EXTRA_OPTIONS_GEOCODER:-} \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${STORE_IMPORTED}:/import:z" \
  -v "${SCRIPT_DIR}/hierarchy:/app/hierarchy:z" \
  -e GEOCODER_IMPORTER_POSTGRES="postgresql://postgres:${NOMINATIM_PASSWORD}@${NOMINATIM_DATABASE_SERVER}/nominatim" \
  -e GEOCODER_JOBS="${GEOCODER_JOBS}" \
  "$GEOCODER_IMAGE"

# nominatim not needed anymore
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

  wait_for_container_shutdown "$DB_CONTAINER" "$NOMINATIM_SHUTDOWN_TIMEOUT"
fi

message "Preparing Valhalla packs for postprocessing..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-valhalla-packs" \
  -v "${STORE_VALHALLA}:/custom_files:z" \
  "$VALHALLA_TILES2PACKS_IMAGE"

message "Running import postprocessing..."
podman run --rm \
  --pod "$POD_NAME" \
  --name "${POD_NAME}-postprocess" \
  --memory="${RAM_DEFALT_LIMIT}" \
  ${PODMAN_EXTRA_OPTIONS_GEOCODER:-} \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${STORE_MBTILES}:/mapbox-planet:z" \
  -v "${STORE_VALHALLA}:/valhalla:z" \
  -v "${STORE_IMPORTED}:/import:z" \
  -v "${STORE_MISC}:/osmscout:z" \
  -v "${SCRIPT_DIR}/hierarchy:/app/hierarchy:z" \
  -v "${SCRIPT_DIR}/provided:/app/provided:z" \
  "$POSTPROCESS_IMAGE"

message "OSM Scout import completed."
