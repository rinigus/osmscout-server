# set version
isEmpty(VERSION) {
    VERSION = 1.17.1
}

# Find out flavor and add it to CONFIG for simple testing
equals(SCOUT_FLAVOR, "silica") {
    CONFIG += scout_silica
} else:equals(SCOUT_FLAVOR, "kirigami") {
    CONFIG += scout_kirigami
} else:equals(SCOUT_FLAVOR, "qtcontrols") {
    CONFIG += scout_qtcontrols
} else:equals(SCOUT_FLAVOR, "uuitk") {
    CONFIG += scout_uuitk
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

# Overall CONFIG
CONFIG += c++14 object_parallel_to_source
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

# defines
scout_silica {
    DEFINES += IS_SAILFISH_OS
} else:scout_qtcontrols|scout_kirigami {
    DEFINES += IS_QTCONTROLS_QT
} else:scout_uuitk {
    DEFINES += IS_QTCONTROLS_QT IS_UUITK
} else {
    DEFINES += IS_CONSOLE_QT
}

scout_uuitk {
    DEFINES += DEFAULT_FALLBACK_STYLE=\\\"suru\\\"
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

