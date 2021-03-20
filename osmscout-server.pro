TEMPLATE = subdirs
SUBDIRS = server

include(common.pri)

# installs
styles.files = styles
styles.path = $$PREFIX/share/$${TARGET}
INSTALLS += styles

data.files = data
data.path = $${PREFIX}/share/$${TARGET}
INSTALLS += data

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

scout_kirigami|scout_qtcontrols|scout_uuitk {
    icons108.path = $$PREFIX/share/icons/hicolor/108x108/apps
    icons108.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/108x108/apps && cp $$PWD/icons/108x108/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/108x108/apps/$${TARGET}.png
    INSTALLS += icons108
    icons128.path = $$PREFIX/share/icons/hicolor/128x128/apps
    icons128.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/128x128/apps && cp $$PWD/icons/128x128/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/128x128/apps/$${TARGET}.png
    INSTALLS += icons128
    icons256.path = $$PREFIX/share/icons/hicolor/256x256/apps
    icons256.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/256x256/apps && cp $$PWD/icons/256x256/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/256x256/apps/$${TARGET}.png
    INSTALLS += icons256
    icons86.path = $$PREFIX/share/icons/hicolor/86x86/apps
    icons86.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/86x86/apps && cp $$PWD/icons/86x86/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/86x86/apps/$${TARGET}.png
    INSTALLS += icons86

    appdata.path =$$PREFIX/share/metainfo
    appdata.files = packaging/osmscout-server.appdata.xml
    INSTALLS += appdata
}

scout_kirigami|scout_qtcontrols {
    desktopfile.path = $$PREFIX/share/applications
    desktopfile.files = $${TARGET}.desktop
    INSTALLS += desktopfile
}

## sources
#SOURCES += src/dbmaster.cpp \
#    src/main.cpp \
#    src/requestmapper.cpp \
#    src/appsettings.cpp \
#    src/dbmaster_route.cpp \
#    src/dbmaster_search.cpp \
#    src/dbmaster_map.cpp \
#    src/osmscout-server.cpp \
#    src/searchresults.cpp \
#    src/infohub.cpp \
#    src/rollinglogger.cpp \
#    src/consolelogger.cpp \
#    src/routingforhuman.cpp \
#    src/geomaster.cpp \
#    src/config.cpp \
#    src/mapmanager.cpp \
#    src/filedownloader.cpp \
#    src/mapmanagerfeature.cpp \
#    src/mapmanagerfeature_packtaskworker.cpp \
#    src/mapnikmaster.cpp \
#    src/valhallamaster.cpp \
#    src/mapmanager_deleterthread.cpp \
#    src/modulechecker.cpp \
#    src/systemdservice.cpp \
#    src/util.cpp \
#    src/mapboxglmaster.cpp \
#    src/mapmanager_urlcollection.cpp \
#    src/valhallamapmatcher.cpp \
#    src/valhallamapmatcherdbus.cpp \
#    src/valhallamapmatcherdbusadaptor.cpp \
#    src/dbusroot.cpp

#HEADERS += \
#    src/dbmaster.h \
#    src/requestmapper.h \
#    src/appsettings.h \
#    src/config.h \
#    src/searchresults.h \
#    src/infohub.h \
#    src/rollinglogger.h \
#    src/consolelogger.h \
#    src/routingforhuman.h \
#    src/geomaster.h \
#    src/mapmanager.h \
#    src/filedownloader.h \
#    src/mapmanagerfeature.h \
#    src/mapmanagerfeature_packtaskworker.h \
#    src/mapnikmaster.h \
#    src/valhallamaster.h \
#    src/mapmanager_deleterthread.h \
#    src/modulechecker.h \
#    src/systemdservice.h \
#    src/util.hpp \
#    src/mapboxglmaster.h \
#    src/mapmanager_urlcollection.h \
#    src/valhallamapmatcher.h \
#    src/valhallamapmatcherdbus.h \
#    src/valhallamapmatcherdbusadaptor.h \
#    src/dbusroot.h

#OTHER_FILES += rpm/osmscout-server.spec
#OTHER_FILES += qml/*.qml
#OTHER_FILES += qml/platform.qtcontrols/*.qml
#OTHER_FILES += qml/platform.kirigami/*.qml
#OTHER_FILES += qml/platform.click/*.qml
#OTHER_FILES += qml/platform.silica/*.qml

## includes
#include(src/uhttp/uhttp.pri)
#include(src/geocoder-nlp/geocoder-nlp.pri)
#!scout_console {
#    include(src/fileselector/fileselector.pri)
#}

## LINKING and backend settings
#use_osmscout {
#    DEFINES += USE_OSMSCOUT

#    use_map_qt {
#        DEFINES += USE_OSMSCOUT_MAP_QT
#        LIBS += -losmscout_map_qt
#    }

#    use_map_cairo {
#        DEFINES += USE_OSMSCOUT_MAP_CAIRO
#        LIBS += -losmscout_map_cairo
#        PKGCONFIG += pango cairo
#    }

#    LIBS += -losmscout_map -losmscout

#    !scout_silica {
#        QMAKE_CXXFLAGS += -fopenmp
#        LIBS += -fopenmp
#    }
#}

## geocoder-nlp is enabled always
#DEFINES += GEOCODERNLP_ALIASFILE=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/data/geocoder-npl-tag-aliases.json\\\"

## mapbox gl is enabled always
#DEFINES += MAPBOXGL_STYLEDIR=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/styles/mapboxgl\\\"

#use_mapnik {
#    DEFINES += USE_MAPNIK
#    #DEFINES += MAPNIK_FONTS_DIR=\\\"$$system(mapnik-config --fonts)\\\"
#    scout_silica {
#        DEFINES += MAPNIK_FONTS_DIR=\\\"/usr/share/harbour-osmscout-server-module-fonts/fonts\\\"
#        DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"/usr/share/$${TARGET}/lib/mapnik/input\\\"
#    } else {
#        DEFINES += MAPNIK_FONTS_DIR=\\\"$${PREFIX_RUNNING}/share/osmscout-server-fonts/fonts\\\"
#        DEFINES += MAPNIK_INPUT_PLUGINS_DIR=\\\"$$system(mapnik-config --input-plugins)\\\"
#    }
#    LIBS += -lmapnik -licuuc

#    scout_uuitk {
#        LIBS += -lproj
#    }
#}

#use_valhalla {
#    DEFINES += USE_VALHALLA
#    DEFINES += VALHALLA_CONFIG_TEMPLATE=\\\"$${PREFIX_RUNNING}/share/$${TARGET}/data/valhalla.json\\\"
#    PKGCONFIG += libvalhalla
#}

#use_curl {
#    DEFINES += USE_LIBCURL
#    PKGCONFIG += libcurl
#}

#use_systemd {
#    DEFINES += USE_SYSTEMD
#    PKGCONFIG += libsystemd
#}

#LIBS += -L$${PREFIX}/lib -lmarisa -lkyotocabinet -lz -lsqlite3

# debug options
CONFIG(release, debug|release) {
    DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
}


# translations
TRANSLATIONS += \
    translations/$${TARGET}-cs.ts \
    translations/$${TARGET}-de.ts \
    translations/$${TARGET}-es.ts \
    translations/$${TARGET}-et.ts \
    translations/$${TARGET}-fi.ts \
    translations/$${TARGET}-fr.ts \
    translations/$${TARGET}-nb.ts \
    translations/$${TARGET}-nl.ts \
    translations/$${TARGET}-pl.ts \
    translations/$${TARGET}-ru.ts \
    translations/$${TARGET}-sv.ts \
    translations/$${TARGET}-nl_BE.ts \
    translations/$${TARGET}-it_IT.ts \
    translations/$${TARGET}-pt_BR.ts

scout_kirigami|scout_qtcontrols {
    CONFIG += lrelease embed_translations
}

scout_uuitk {
    qtPrepareTool(LRELEASE, lrelease)
    for(tsfile, TRANSLATIONS) {
        qmfile = $$shadowed($$tsfile)
        qmfile ~= s,.ts$,.qm,
        qmdir = $$dirname(qmfile)
        !exists($$qmdir) {
            mkpath($$qmdir)|error("Aborting.")
        }
        command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
        system($$command)|error("Failed to run: $$command")
        TRANSLATIONS_FILES += $$qmfile
    }
}

DISTFILES += $${TARGET}.desktop

scout_silica {
    DISTFILES += \
        rpm/$${TARGET}.spec
}

scout_kirigami|scout_qtcontrols|scout_uuitk {
    RESOURCES += qml_main.qrc
    RESOURCES += icons.qrc
}
scout_kirigami: RESOURCES += qml_kirigami.qrc
scout_qtcontrols: RESOURCES += qml_qtcontrols.qrc
scout_uuitk: RESOURCES += qml_uuitk.qrc

# misc options
scout_silica {
    SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256
}

