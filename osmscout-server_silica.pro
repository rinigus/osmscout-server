# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-osmscout-server

QT += core gui network sql xml

CONFIG += c++11 object_parallel_to_source
CONFIG += sailfishapp sailfishapp_no_deploy_qml

# selection of backends
!disable_mapnik {
    CONFIG += use_mapnik
}

!disable_osmscout {
    CONFIG += use_osmscout
}

!disable_valhalla {
    CONFIG += use_valhalla
}

!disable_systemd {
    CONFIG += use_systemd
}

# libosmscout settings
!use_map_cairo {
    CONFIG += use_map_qt
}
#CONFIG += use_map_cairo

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# installs
qml.files = qml/silica
qml.path = /usr/share/$${TARGET}/qml
INSTALLS += qml

data.files = data
data.path = /usr/share/$${TARGET}
INSTALLS += data

styles.files = styles
styles.path = /usr/share/$${TARGET}
INSTALLS += styles

# defines
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += IS_SAILFISH_OS

SOURCES += \
    src/dbmaster.cpp \
    src/main.cpp \
    src/requestmapper.cpp \
    src/appsettings.cpp \
    src/dbmaster_route.cpp \
    src/dbmaster_search.cpp \
    src/dbmaster_map.cpp \
    src/osmscout-server_silica.cpp \
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
    src/mapmanager_urlcollection.cpp \
    src/mapmanagerfeature_packtaskworker.cpp \
    src/mapnikmaster.cpp \ 
    src/modulechecker.cpp \
    src/valhallamaster.cpp \
    src/mapmanager_deleterthread.cpp \
    src/systemdservice.cpp \
    src/util.cpp \
    src/mapboxglmaster.cpp 
#    src/sqlite/sqlite-amalgamation-3160200/sqlite3.c

OTHER_FILES += rpm/osmscout-server.spec

include(src/uhttp/uhttp.pri)
include(src/fileselector/fileselector.pri)
include(src/geocoder-nlp/geocoder-nlp.pri)

HEADERS += \
    src/dbmaster.h \
    src/requestmapper.h \
    src/appsettings.h \
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
    src/mapmanager_urlcollection.h \
    src/mapmanagerfeature_packtaskworker.h \
    src/mapnikmaster.h \
    src/modulechecker.h \
    src/valhallamaster.h \
    src/mapmanager_deleterthread.h \
    src/systemdservice.h \
    src/util.hpp \
    src/mapboxglmaster.h
#    src/sqlite/sqlite-amalgamation-3160200/sqlite3.h \
#    src/sqlite/sqlite-amalgamation-3160200/sqlite3ext.h

use_osmscout {
    DEFINES += USE_OSMSCOUT

    use_map_qt {
        DEFINES += USE_OSMSCOUT_MAP_QT
        QT += gui
        LIBS += -losmscout_map_qt
    }

    use_map_cairo {
        DEFINES += USE_OSMSCOUT_MAP_CAIRO
        LIBS += -losmscout_map_cairo
        PKGCONFIG += pango cairo
    }

    LIBS += -losmscout_map -losmscout
}

# geocoder-nlp is enabled always
DEFINES += GEOCODERNLP_ALIASFILE=\\\"/usr/share/$${TARGET}/data/geocoder-npl-tag-aliases.json\\\"

# mapbox gl is enabled always
DEFINES += MAPBOXGL_STYLEDIR=\\\"/usr/share/$${TARGET}/styles/mapboxgl\\\"

use_mapnik {
    DEFINES += USE_MAPNIK
    DEFINES += MAPNIK_FONTS_DIR=\\\"/usr/share/harbour-osmscout-server-module-fonts/fonts\\\"
    DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"/usr/share/$${TARGET}/lib/mapnik/input\\\"
    LIBS += -lmapnik -licuuc

    mapnik.files = mapnik
    mapnik.path = /usr/share/$${TARGET}
    INSTALLS += mapnik
}

use_valhalla {
    DEFINES += USE_VALHALLA
    DEFINES += VALHALLA_EXECUTABLE=\\\"/usr/bin/harbour-osmscout-server-module-route\\\"
    DEFINES += VALHALLA_CONFIG_TEMPLATE=\\\"/usr/share/harbour-osmscout-server-module-route/data/valhalla.json\\\"
    CONFIG += use_curl
}

use_systemd {
    DEFINES += USE_SYSTEMD
    PKGCONFIG += libsystemd-daemon
}

use_curl {
    DEFINES += USE_LIBCURL
    PKGCONFIG += libcurl
}

LIBS += -lmarisa -lkyotocabinet -lz -ldl
LIBS += -lsqlite3

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

CONFIG(release, debug|release) {
    DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
}


TRANSLATIONS += \
    translations/harbour-osmscout-server-cs.ts \
    translations/harbour-osmscout-server-de.ts \
    translations/harbour-osmscout-server-es.ts \
    translations/harbour-osmscout-server-fi.ts \
    translations/harbour-osmscout-server-fr.ts \
    translations/harbour-osmscout-server-nb.ts \
    translations/harbour-osmscout-server-nl.ts \
    translations/harbour-osmscout-server-pl.ts \
    translations/harbour-osmscout-server-ru.ts \
    translations/harbour-osmscout-server-sv.ts \
    translations/harbour-osmscout-server-nl_BE.ts

DISTFILES += \
    harbour-osmscout-server.desktop \
    rpm/harbour-osmscout-server.yaml \
    rpm/harbour-osmscout-server.changes \
    rpm/harbour-osmscout-server.spec

