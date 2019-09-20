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

# set version
isEmpty(VERSION) {
    VERSION = 1.14.5
}

# Find out flavor and add it to CONFIG for simple testing
equals(SCOUT_FLAVOR, "silica") {
    CONFIG += scout_silica
} else:equals(SCOUT_FLAVOR, "kirigami") {
    CONFIG += scout_kirigami
} else:equals(SCOUT_FLAVOR, "qtcontrols") {
    CONFIG += scout_qtcontrols
} else {
    CONFIG += scout_console
}

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

############################################
# Below, configuration is set on the basis
# of the options specified above

# The name of the application
isEmpty(APP_NAME) {
    scout_silica {
        APP_NAME = harbour-osmscout-server
    } else {
        APP_NAME = osmscout-server
    }
}

TARGET=$${APP_NAME}

# Overall QT options
QT += core network sql xml positioning dbus

!scout_console|use_map_qt {
    QT += gui
}
scout_kirigami|scout_qtcontrols {
    QT += quick qml widgets
}

# Overall CONFIG
CONFIG += c++11 object_parallel_to_source
CONFIG += link_pkgconfig
scout_silica {
    CONFIG += sailfishapp sailfishapp_no_deploy_qml sailfishapp_i18n
}

# PREFIX
isEmpty(PREFIX) {
    scout_silica {
        PREFIX = /usr
    } else {
        PREFIX = /usr/local
    }
}

# installs
styles.files = styles
styles.path = $$PREFIX/share/$${TARGET}
INSTALLS += styles

data.files = data
data.path = $${PREFIX}/share/$${TARGET}
INSTALLS += data

target.path = $$PREFIX/bin
INSTALLS += target

scout_silica {
    qml.files = qml/*.qml
    qml.path = /usr/share/$${TARGET}/qml
    INSTALLS += qml

    qmlplatform.files = qml/platform.silica/*.qml
    qmlplatform.path = /usr/share/$${TARGET}/qml/platform
    INSTALLS += qmlplatform

    extra_icons.files = icons/banner.jpg icons/osmscout-server.svg
    extra_icons.path = /usr/share/$${TARGET}/icons
    INSTALLS += extra_icons
}

scout_kirigami|scout_qtcontrols {
    icons108.path = $$PREFIX/share/icons/hicolor/108x108/apps
    icons108.extra = cp $$PWD/icons/108x108/harbour-osmscout-server.png $(INSTALL_ROOT)$$PREFIX/share/icons/hicolor/108x108/apps/$${TARGET}.png
    INSTALLS += icons108
    icons128.path = $$PREFIX/share/icons/hicolor/128x128/apps
    icons128.extra = cp $$PWD/icons/128x128/harbour-osmscout-server.png $(INSTALL_ROOT)$$PREFIX/share/icons/hicolor/128x128/apps/$${TARGET}.png
    INSTALLS += icons128
    icons256.path = $$PREFIX/share/icons/hicolor/256x256/apps
    icons256.extra = cp $$PWD/icons/256x256/harbour-osmscout-server.png $(INSTALL_ROOT)$$PREFIX/share/icons/hicolor/256x256/apps/$${TARGET}.png
    INSTALLS += icons256
    icons86.path = $$PREFIX/share/icons/hicolor/86x86/apps
    icons86.extra = cp $$PWD/icons/86x86/harbour-osmscout-server.png $(INSTALL_ROOT)$$PREFIX/share/icons/hicolor/86x86/apps/$${TARGET}.png
    INSTALLS += icons86

    appdata.path =$$PREFIX/share/metainfo
    appdata.files = packaging/osmscout-server.appdata.xml
    INSTALLS += appdata
}

# defines
DEFINES += APP_NAME=\\\"$$APP_NAME\\\"
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
scout_silica {
    DEFINES += IS_SAILFISH_OS
} else:scout_qtcontrols|scout_kirigami {
    DEFINES += IS_QTCONTROLS_QT
} else {
    DEFINES += IS_CONSOLE_QT
}

# default prefix for data
DEFINES += DEFAULT_DATA_PREFIX=\\\"$${PREFIX}/share/$${TARGET}/\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# sources
SOURCES += src/dbmaster.cpp \
    src/main.cpp \
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
OTHER_FILES += qml/platform.silica/*.qml

# includes
include(src/uhttp/uhttp.pri)
include(src/geocoder-nlp/geocoder-nlp.pri)
!scout_console {
    include(src/fileselector/fileselector.pri)
}

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
DEFINES += GEOCODERNLP_ALIASFILE=\\\"$${PREFIX}/share/$${TARGET}/data/geocoder-npl-tag-aliases.json\\\"

# mapbox gl is enabled always
DEFINES += MAPBOXGL_STYLEDIR=\\\"$${PREFIX}/share/$${TARGET}/styles/mapboxgl\\\"

use_mapnik {
    DEFINES += USE_MAPNIK
    #DEFINES += MAPNIK_FONTS_DIR=\\\"$$system(mapnik-config --fonts)\\\"
    scout_silica {
        DEFINES += MAPNIK_FONTS_DIR=\\\"/usr/share/harbour-osmscout-server-module-fonts/fonts\\\"
        DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"/usr/share/$${TARGET}/lib/mapnik/input\\\"
    } else {
        DEFINES += MAPNIK_FONTS_DIR=\\\"$${PREFIX}/share/osmscout-server-fonts/fonts\\\"
        DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"$$system(mapnik-config --input-plugins)\\\"
    }
    LIBS += -lmapnik -licuuc
}

use_valhalla {
    DEFINES += USE_VALHALLA
    DEFINES += VALHALLA_CONFIG_TEMPLATE=\\\"$${PREFIX}/share/$${TARGET}/data/valhalla.json\\\"
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

# debug options
CONFIG(release, debug|release) {
    DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
}


# translations
TRANSLATIONS += \
    translations/$${TARGET}-cs.ts \
    translations/$${TARGET}-de.ts \
    translations/$${TARGET}-es.ts \
    translations/$${TARGET}-fi.ts \
    translations/$${TARGET}-fr.ts \
    translations/$${TARGET}-nb.ts \
    translations/$${TARGET}-nl.ts \
    translations/$${TARGET}-pl.ts \
    translations/$${TARGET}-ru.ts \
    translations/$${TARGET}-sv.ts \
    translations/$${TARGET}-nl_BE.ts \
    translations/$${TARGET}-it_IT.ts

DISTFILES += $${TARGET}.desktop

scout_silica {
    DISTFILES += \
        rpm/$${TARGET}.yaml \
        rpm/$${TARGET}.changes \
        rpm/$${TARGET}.spec
}

scout_kirigami|scout_qtcontrols {
    RESOURCES += qml_main.qrc
    RESOURCES += icons.qrc
}
scout_kirigami: RESOURCES += qml_kirigami.qrc
scout_qtcontrols: RESOURCES += qml_qtcontrols.qrc


# misc options
scout_silica {
    SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256
    LIBS += -L/opt/gcc6/lib -static-libstdc++
}

