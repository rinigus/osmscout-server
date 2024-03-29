include(../common.pri)

TARGET=$${APP_NAME}-gui

# defines
DEFINES += APP_NAME=\\\"$$APP_NAME\\\"
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += gui

scout_kirigami|scout_qtcontrols|scout_uuitk|scout_lomiri {
    QT += quick qml widgets quickcontrols2
}

target.path = $$SERVER_PATH
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

icons108.path = $$PREFIX/share/icons/hicolor/108x108/apps
icons108.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/108x108/apps && cp $$PWD/icons/108x108/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/108x108/apps/$${APP_NAME}.png
INSTALLS += icons108
icons128.path = $$PREFIX/share/icons/hicolor/128x128/apps
icons128.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/128x128/apps && cp $$PWD/icons/128x128/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/128x128/apps/$${APP_NAME}.png
INSTALLS += icons128
icons256.path = $$PREFIX/share/icons/hicolor/256x256/apps
icons256.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/256x256/apps && cp $$PWD/icons/256x256/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/256x256/apps/$${APP_NAME}.png
INSTALLS += icons256
icons86.path = $$PREFIX/share/icons/hicolor/86x86/apps
icons86.extra = mkdir -p $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/86x86/apps && cp $$PWD/icons/86x86/harbour-osmscout-server.png $(INSTALL_ROOT)/$$PREFIX/share/icons/hicolor/86x86/apps/$${APP_NAME}.png
INSTALLS += icons86

scout_kirigami|scout_qtcontrols|scout_uuitk|scout_lomiri {
    appdata.path =$$PREFIX/share/metainfo
    appdata.files = packaging/osmscout-server.appdata.xml
    INSTALLS += appdata
}

desktopfile.path = $$PREFIX/share/applications
desktopfile.files = $${TARGET}.desktop
INSTALLS += desktopfile

# sources
SOURCES += \
    src/geomaster.cpp \
    src/infohub.cpp \
    src/logger.cpp \
    src/main.cpp \
    src/appsettings.cpp \
    src/mapmanager.cpp \
    src/modulechecker.cpp \
    src/servercontroller.cpp \
    src/serverdbusroot.cpp \
    src/systemdservice.cpp \
    src/trackdbusservice.cpp

HEADERS += \
    src/config-common.h \
    src/appsettings.h \
    src/geomaster.h \
    src/infohub.h \
    src/logger.h \
    src/mapmanager.h \
    src/modulechecker.h \
    src/servercontroller.h \
    src/serverdbusroot.h \
    src/systemdservice.h \
    src/trackdbusservice.h

OTHER_FILES += qml/*.qml
OTHER_FILES += qml/platform.qtcontrols/*.qml
OTHER_FILES += qml/platform.kirigami/*.qml
OTHER_FILES += qml/platform.click/*.qml
OTHER_FILES += qml/platform.silica/*.qml
OTHER_FILES += qml/platform.uuitk/*.qml
OTHER_FILES += qml/platform.lomiri/*.qml
OTHER_FILES += osmscout-server.desktop harbour-osmscout-server.desktop

include(src/fileselector/fileselector.pri)

DISTFILES += $${TARGET}.desktop

scout_kirigami|scout_qtcontrols|scout_uuitk|scout_lomiri {
    RESOURCES += qml_main.qrc
    RESOURCES += icons.qrc
}
scout_kirigami: RESOURCES += qml_kirigami.qrc
scout_qtcontrols: RESOURCES += qml_qtcontrols.qrc
scout_uuitk: RESOURCES += qml_uuitk.qrc
scout_lomiri: RESOURCES += qml_lomiri.qrc

# translations
TRANSLATIONS += \
    ../translations/$${APP_NAME}-cs.ts \
    ../translations/$${APP_NAME}-de.ts \
    ../translations/$${APP_NAME}-es.ts \
    ../translations/$${APP_NAME}-et.ts \
    ../translations/$${APP_NAME}-fi.ts \
    ../translations/$${APP_NAME}-fr.ts \
    ../translations/$${APP_NAME}-nb.ts \
    ../translations/$${APP_NAME}-nl.ts \
    ../translations/$${APP_NAME}-pl.ts \
    ../translations/$${APP_NAME}-ru.ts \
    ../translations/$${APP_NAME}-sv.ts \
    ../translations/$${APP_NAME}-nl_BE.ts \
    ../translations/$${APP_NAME}-it_IT.ts \
    ../translations/$${APP_NAME}-pt_BR.ts

scout_kirigami|scout_qtcontrols {
    CONFIG += lrelease embed_translations
}

# set translation folder loaded by application
scout_kirigami|scout_qtcontrols {
    DEFINES += TRANSLATION_FOLDER=\\\":/i18n\\\"
}

scout_silica {
    DEFINES += TRANSLATION_FOLDER=\\\"$${PREFIX}/share/$${APP_NAME}/translations\\\"
}

scout_uuitk|scout_lomiri {
    DEFINES += TRANSLATION_FOLDER=\\\"./translations\\\"
}

# platform specific
scout_silica {
    CONFIG += sailfishapp sailfishapp_no_deploy_qml
}
