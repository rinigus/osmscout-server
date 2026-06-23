#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/podman-import-common.sh"

load_import_env .env

NAMEBASE="osmscout-download"

# download pbf
if [ -f "${STORE_PLANET}/${PBF}" ]; then
  message "Area PBF download skipped: ${STORE_PLANET}/${PBF} is already available."
else
  message "Downloading area PBF..."
  podman run --rm -it \
    --name "${NAMEBASE}-pbf" \
    -v "${STORE_PLANET}:/planet_pbf:z" \
    "${OSMDOWNLOAD_IMAGE}" download-osm ${AREA} -o /planet_pbf/${PBF}
fi

# download data for nominatim
message "Downloading Nominatim auxiliary data..."
podman run --rm \
  --name "${NAMEBASE}-nominatim" \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${SCRIPT_DIR}/scripts:/scripts:z" \
  "$WGET_IMAGE" \
  /scripts/get_urls.sh /planet_pbf \
    https://nominatim.org/data/wikimedia-importance.sql.gz \
    https://nominatim.org/data/gb_postcodes.csv.gz \
    https://nominatim.org/data/us_postcodes.csv.gz

# download data for libpostal
message "Downloading Libpostal data..."
podman run --rm \
  --name "${NAMEBASE}-geocoder" \
  -v "${STORE_PLANET}:/planet_pbf:z" \
  -v "${SCRIPT_DIR}/scripts:/scripts:z" \
  "$GEOCODER_IMAGE" \
  /app/entrypoint.sh --download
