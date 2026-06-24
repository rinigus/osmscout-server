# build
FROM debian:bookworm AS build

RUN apt-get update
RUN apt-get install -y\
    git make cmake \
    g++ gcc \
    autoconf libtool \
    nlohmann-json3-dev \
    libmarisa-dev libkyotocabinet-dev \
    libsqlite3-dev libpqxx-dev \
    libboost-program-options-dev

WORKDIR /app

# libpostal
RUN git clone https://github.com/openvenues/libpostal.git
WORKDIR /app/libpostal
RUN git checkout 8f2066b1d30f4290adf59cacc429980f139b8545

RUN ./bootstrap.sh
RUN ./configure \
    --prefix=/usr \
    --datadir=/planet_pbf/libpostal \
    --disable-data-download
RUN make -j${nproc} && make install

WORKDIR /app

# geocoder-nlp
RUN git clone https://github.com/rinigus/geocoder-nlp.git
WORKDIR /app/geocoder-nlp
RUN git checkout 2c9fc2bd2f51ca2c36b1266bf5b58db593b5dd4d
RUN git submodule init && \
    git submodule update

RUN cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr
RUN make -C build -j${nproc} && \
    make -C build install

WORKDIR /app

#############
# runtime
FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y \
    python3 python-is-python3 \
    bzip2 gzip \
    curl

# copy from build
COPY --from=build /usr/bin/geocoder-importer /usr/bin/
COPY --from=build /app/geocoder-nlp/importer/data /app/geocoder-nlp
COPY --from=build /usr/bin/libpostal_data /usr/bin/
COPY --from=build /usr/lib/libpostal.so* /usr/lib/

RUN apt-get install -y \
    make \
    libmarisa0 \
    libkyotocabinet16v5 \
    libpqxx-6.4 \
    libboost-program-options1.74.0

WORKDIR /app

# copy used scripts
COPY ./geocoder/entrypoint.sh ./pack.sh /app
COPY ./geocoder/build_geocoder.sh /app/build_geocoder.sh

COPY ./hierarchy.py /app
COPY ./geocoder/prepare_import.py /app

ENTRYPOINT ["/app/entrypoint.sh"]
