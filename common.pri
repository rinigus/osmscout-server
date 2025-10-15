# Find out flavor and add it to CONFIG for simple testing
equals(SCOUT_FLAVOR, "silica") {
    CONFIG += scout_silica
} else:equals(SCOUT_FLAVOR, "kirigami") {
    CONFIG += scout_kirigami
} else:equals(SCOUT_FLAVOR, "qtcontrols") {
    CONFIG += scout_qtcontrols
} else:equals(SCOUT_FLAVOR, "uuitk") {
    CONFIG += scout_uuitk
} else:equals(SCOUT_FLAVOR, "lomiri") {
    CONFIG += scout_lomiri
} else {
    error("Specify SCOUT_FLAVOR during configuration")
}

# set version
isEmpty(VERSION) {
    VERSION = 3.1.6
}

# The name of the application
isEmpty(APP_NAME) {
    scout_silica {
        APP_NAME = harbour-osmscout-server
    } else {
        APP_NAME = osmscout-server
    }
}

# Overall QT options
QT += core network dbus

# selection of backends
!disable_mapnik {
    CONFIG += use_mapnik
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

# Overall CONFIG
CONFIG += object_parallel_to_source
CONFIG += link_pkgconfig

# PREFIX
isEmpty(PREFIX) {
    scout_silica {
        PREFIX = /usr
    } else {
        PREFIX = /usr/local
    }
}

# PREFIX_RUNNING
isEmpty(PREFIX_RUNNING) {
    PREFIX_RUNNING = $$PREFIX
}

SERVER_PATH=$$PREFIX/bin
SERVER_EXE=$${PREFIX_RUNNING}/bin/$${APP_NAME}
DEFINES += SERVER_EXE=\\\"$${SERVER_EXE}\\\"

# defines
scout_silica {
    DEFINES += IS_SAILFISH_OS
} else:scout_qtcontrols|scout_kirigami {
    DEFINES += IS_QTCONTROLS_QT
} else:scout_uuitk {
    DEFINES += IS_QTCONTROLS_QT IS_UUITK
} else:scout_lomiri {
    DEFINES += IS_QTCONTROLS_QT IS_LOMIRI
} else {
    DEFINES += IS_CONSOLE_QT
}

scout_uuitk|scout_lomiri {
    DEFINES += DEFAULT_FALLBACK_STYLE=\\\"suru\\\"
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# debug options
CONFIG(release, debug|release) {
    DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
}

