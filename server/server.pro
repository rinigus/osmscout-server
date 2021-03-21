include(../common.pri)

TARGET=$${APP_NAME}

# Overall QT options
QT += sql xml positioning

target.path = $$PREFIX/bin
INSTALLS += target

# Installing dbus service, processing path
use_dbusactivation {
    dbusact.extra = install -v -m 644 $$PWD/server/org.osm.scout.server1.service \
       ${INSTALL_ROOT}/usr/share/dbus-1/services/org.osm.scout.server1.service ;\
       sed -i -e \'s|PREFIX|$${PREFIX}|g\' ${INSTALL_ROOT}/usr/share/dbus-1/services/org.osm.scout.server1.service;
    dbusact.path = /usr/share/dbus-1/services
    INSTALLS += dbusact
}

# defines
DEFINES += APP_NAME=\\\"$$APP_NAME\\\"
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# default prefix for data
DEFINES += DEFAULT_DATA_PREFIX=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# sources
SOURCES += src/dbmaster.cpp \
    src/main.cpp \
    src/mapmanagerdbusadaptor.cpp \
    src/requestmapper.cpp \
    src/appsettings.cpp \
    src/dbmaster_route.cpp \
    src/dbmaster_search.cpp \
    src/dbmaster_map.cpp \
    src/osmscout-server.cpp \
    src/searchresults.cpp \
    src/infohub.cpp \
    src/rollinglogger.cpp \
    src/consolelogger.cpp \
    src/routingforhuman.cpp \
    src/geomaster.cpp \
    src/config.cpp \
    src/mapmanager.cpp \
    src/filedownloader.cpp \
    src/mapmanagerfeature.cpp \
    src/mapmanagerfeature_packtaskworker.cpp \
    src/mapnikmaster.cpp \
    src/valhallamaster.cpp \
    src/mapmanager_deleterthread.cpp \
    src/modulechecker.cpp \
    src/systemdservice.cpp \
    src/util.cpp \
    src/mapboxglmaster.cpp \
    src/mapmanager_urlcollection.cpp \
    src/valhallamapmatcher.cpp \
    src/valhallamapmatcherdbus.cpp \
    src/valhallamapmatcherdbusadaptor.cpp \
    src/dbusroot.cpp

HEADERS += \
    src/dbmaster.h \
    src/mapmanagerdbusadaptor.h \
    src/requestmapper.h \
    src/appsettings.h \
    src/config-common.h \
    src/config.h \
    src/searchresults.h \
    src/infohub.h \
    src/rollinglogger.h \
    src/consolelogger.h \
    src/routingforhuman.h \
    src/geomaster.h \
    src/mapmanager.h \
    src/filedownloader.h \
    src/mapmanagerfeature.h \
    src/mapmanagerfeature_packtaskworker.h \
    src/mapnikmaster.h \
    src/valhallamaster.h \
    src/mapmanager_deleterthread.h \
    src/modulechecker.h \
    src/systemdservice.h \
    src/util.hpp \
    src/mapboxglmaster.h \
    src/mapmanager_urlcollection.h \
    src/valhallamapmatcher.h \
    src/valhallamapmatcherdbus.h \
    src/valhallamapmatcherdbusadaptor.h \
    src/dbusroot.h

OTHER_FILES += rpm/osmscout-server.spec
OTHER_FILES += qml/*.qml
OTHER_FILES += qml/platform.qtcontrols/*.qml
OTHER_FILES += qml/platform.kirigami/*.qml
OTHER_FILES += qml/platform.click/*.qml
OTHER_FILES += qml/platform.silica/*.qml

# includes
include(src/uhttp/uhttp.pri)
include(src/geocoder-nlp/geocoder-nlp.pri)

# LINKING and backend settings
use_osmscout {
    DEFINES += USE_OSMSCOUT

    use_map_qt {
        DEFINES += USE_OSMSCOUT_MAP_QT
        LIBS += -losmscout_map_qt
    }

    use_map_cairo {
        DEFINES += USE_OSMSCOUT_MAP_CAIRO
        LIBS += -losmscout_map_cairo
        PKGCONFIG += pango cairo
    }

    LIBS += -losmscout_map -losmscout

    !scout_silica {
        QMAKE_CXXFLAGS += -fopenmp
        LIBS += -fopenmp
    }
}

# geocoder-nlp is enabled always
DEFINES += GEOCODERNLP_ALIASFILE=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/data/geocoder-npl-tag-aliases.json\\\"

# mapbox gl is enabled always
DEFINES += MAPBOXGL_STYLEDIR=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/styles/mapboxgl\\\"

use_mapnik {
    DEFINES += USE_MAPNIK
    #DEFINES += MAPNIK_FONTS_DIR=\\\"$$system(mapnik-config --fonts)\\\"
    scout_silica {
        DEFINES += MAPNIK_FONTS_DIR=\\\"/usr/share/harbour-osmscout-server-module-fonts/fonts\\\"
        DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"/usr/share/$${TARGET}/lib/mapnik/input\\\"
    } else {
        DEFINES += MAPNIK_FONTS_DIR=\\\"$${PREFIX_RUNNING}/share/osmscout-server-fonts/fonts\\\"
        DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"$$system(mapnik-config --input-plugins)\\\"
    }
    LIBS += -lmapnik -licuuc

    scout_uuitk {
        LIBS += -lproj
    }
}

use_valhalla {
    DEFINES += USE_VALHALLA
    DEFINES += VALHALLA_CONFIG_TEMPLATE=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/data/valhalla.json\\\"
    PKGCONFIG += libvalhalla
}

use_curl {
    DEFINES += USE_LIBCURL
    PKGCONFIG += libcurl
}

use_systemd {
    DEFINES += USE_SYSTEMD
    PKGCONFIG += libsystemd
}

LIBS += -L$${PREFIX}/lib -lmarisa -lkyotocabinet -lz -lsqlite3
