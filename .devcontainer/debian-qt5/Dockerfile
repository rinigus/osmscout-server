FROM debian:12

ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        libboost-dev \
        libboost-date-time-dev \
        libboost-filesystem-dev \
        libboost-iostreams-dev \
        libboost-program-options-dev \
        libboost-regex-dev \
        libboost-system-dev \
        autoconf \
        automake \
        ca-certificates \
        cmake \
        curl \
        git \
        libtool \
        ninja-build \
        pkg-config \
        qtbase5-dev \
        qtdeclarative5-dev \
        qtpositioning5-dev \
        qtquickcontrols2-5-dev \
        qttools5-dev \
        qttools5-dev-tools \
        qtwayland5 \
        qml-module-org-kde-kirigami2 \
        qml-module-qt-labs-settings \
        qml-module-qtquick-controls2 \
        qml-module-qtquick-dialogs \
        qml-module-qtquick-layouts \
        dbus-x11 \
        libgl1-mesa-dri \
        libmicrohttpd-dev \
        libmarisa-dev \
        libkyotocabinet-dev \
        libcurl4-gnutls-dev \
        liblua5.3-dev \
        liblz4-dev \
        libmapnik-dev \
        locales \
        libprotobuf-dev \
        libsqlite3-dev \
        libsystemd-dev \
        mapnik-utils \
        protobuf-compiler \
        zlib1g-dev \
        sudo \
    && locale-gen it_IT.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

# Compile libpostal from source
RUN git clone --branch 1.0.0 --depth 1 --recurse-submodules --shallow-submodules https://github.com/rinigus/pkg-libpostal.git /tmp/pkg-libpostal \
    && cd /tmp/pkg-libpostal/libpostal \
    && ./bootstrap.sh \
    && ./configure --datadir=/usr/local/share/libpostal --disable-data-download \
    && make -j"$(nproc)" \
    && make install \
    && ldconfig \
    && rm -rf /tmp/pkg-libpostal

# Compile Valhalla from source
RUN git clone --branch 3.4.0.5 --depth 1 --recurse-submodules --shallow-submodules https://github.com/rinigus/pkg-valhalla-lite.git /tmp/pkg-valhalla-lite \
    && cd /tmp/pkg-valhalla-lite/valhalla \
    && cmake -S . -B build -G "Unix Makefiles" \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DENABLE_DATA_TOOLS=OFF \
        -DENABLE_PYTHON_BINDINGS=OFF \
        -DENABLE_SERVICES=OFF \
        -DENABLE_TESTS=OFF \
        -DENABLE_TOOLS=OFF \
        -DENABLE_WERROR=OFF \
        -DENABLE_SINGLE_FILES_WERROR=OFF \
    && cmake --build build --parallel "$(nproc)" \
    && cd build \
    && cmake --install . \
    && cp -r ../third_party/date/include/date /usr/local/include/ \
    && ldconfig \
    && rm -rf /tmp/pkg-valhalla-lite

RUN groupadd --gid "${USER_GID}" "${USERNAME}" \
    && useradd --uid "${USER_UID}" --gid "${USER_GID}" -m -s /bin/bash "${USERNAME}" \
    && echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > "/etc/sudoers.d/${USERNAME}" \
    && chmod 0440 "/etc/sudoers.d/${USERNAME}" \
    && mkdir -p "/tmp/runtime-${USERNAME}" \
    && chown "${USERNAME}:${USERNAME}" "/tmp/runtime-${USERNAME}" \
    && chmod 0700 "/tmp/runtime-${USERNAME}"

ENV LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    QT_X11_NO_MITSHM=1

USER ${USERNAME}
WORKDIR /workspace
