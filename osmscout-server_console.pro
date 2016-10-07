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
TARGET = osmscout-server

QT = core network

CONFIG += c++11

CONFIG += use_map_qt
#CONFIG += use_map_cairo

# installs
stylesheets.files = stylesheets
stylesheets.path = /usr/share/$${TARGET}
INSTALLS += stylesheets

data.files = data
data.path = /usr/share/$${TARGET}
INSTALLS += data

# defines
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += IS_CONSOLE_QT

SOURCES += src/dbmaster.cpp \
    src/main.cpp \
    src/requestmapper.cpp \
    src/appsettings.cpp \
    src/dbmaster_search.cpp \
    src/dbmaster_map.cpp \
    src/osmscout-server_console.cpp

OTHER_FILES += \
    osmscout-server.desktop

include(src/httpserver/httpserver.pri)

LIBS += -losmscout_map -losmscout

HEADERS += \
    src/dbmaster.h \
    src/requestmapper.h \
    src/appsettings.h \
    src/config.h

use_map_qt {
    DEFINES += USE_OSMSCOUT_MAP_QT
    QT += gui
    LIBS += -losmscout_map_qt
}

use_map_cairo {
    DEFINES += USE_OSMSCOUT_MAP_CAIRO
    LIBS += -losmscout_map_cairo
    CONFIG += link_pkgconfig
    PKGCONFIG += pango cairo
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
}
