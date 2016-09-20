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
CONFIG += sailfishapp

stylesheets.files = stylesheets
stylesheets.path = /usr/share/$${TARGET}
INSTALLS += stylesheets

SOURCES += src/osmscout-server.cpp \
    src/dbmaster.cpp \
    src/main.cpp \
    src/requestmapper.cpp

OTHER_FILES += qml/osmscout-server.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/SecondPage.qml \
    rpm/osmscout-server.changes.in \
    rpm/osmscout-server.spec \
    rpm/osmscout-server.yaml \
    translations/*.ts \
    osmscout-server.desktop

OTHER_FILES += stylesheets/*

include(src/httpserver/httpserver.pri)

LIBS += -losmscout_map_qt -losmscout_map -losmscout

HEADERS += \
    src/dbmaster.h \
    src/requestmapper.h

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/osmscout-server-de.ts

