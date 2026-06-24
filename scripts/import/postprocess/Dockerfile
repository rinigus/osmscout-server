FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y \
    python3 python3-venv \
    bzip2 gzip \
    curl

WORKDIR /app

# create and activate venv
RUN python3 -m venv venv
ENV PATH=/app/venv/bin:$PATH

RUN pip install \
    numpy \
    mercantile \
    psutil \
    shapely \
    tabulate

# copy used scripts
COPY \
    ./postprocess/entrypoint.sh \
    ./postprocess/mapbox_packs.sh \
    ./postprocess/mapbox_planetiler_split.py \
    ./postprocess/valhalla_packs.py \
    ./pack.sh \
    /app
COPY ./hierarchy.py /app

COPY \
    ./postprocess/prepare_countries.py \
    ./postprocess/mapbox_country_pack.py \
    ./postprocess/valhalla_country_pack.py \
    /app

# command
CMD /app/entrypoint.sh
