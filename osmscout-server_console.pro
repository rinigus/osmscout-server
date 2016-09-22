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

QT += core gui network

CONFIG += c++11

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

SOURCES += src/osmscout-server.cpp \
    src/dbmaster.cpp \
    src/main.cpp \
    src/requestmapper.cpp \
    src/appsettings.cpp

OTHER_FILES += \
    osmscout-server.desktop

include(src/httpserver/httpserver.pri)

LIBS += -losmscout_map_qt -losmscout_map -losmscout

HEADERS += \
    src/dbmaster.h \
    src/requestmapper.h \
    src/appsettings.h \
    src/config.h

