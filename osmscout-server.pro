TEMPLATE = subdirs
SUBDIRS = server gui

include(common.pri)

TARGET=$${APP_NAME}

# installs
styles.files = styles
styles.path = $$PREFIX/share/$${TARGET}
INSTALLS += styles

geocoder_data.files = data/geocoder-npl-tag-aliases.json
geocoder_data.path = $${PREFIX}/share/$${TARGET}/data
INSTALLS += geocoder_data

valhalla_data.files = data/valhalla.json-*
valhalla_data.path = $${PREFIX}/share/$${TARGET}/data
INSTALLS += valhalla_data

OTHER_FILES += rpm/osmscout-server.spec

# translations
TRANSLATIONS += \
    translations/$${APP_NAME}-cs.ts \
    translations/$${APP_NAME}-de.ts \
    translations/$${APP_NAME}-es.ts \
    translations/$${APP_NAME}-et.ts \
    translations/$${APP_NAME}-fi.ts \
    translations/$${APP_NAME}-fr.ts \
    translations/$${APP_NAME}-it_IT.ts \
    translations/$${APP_NAME}-nb.ts \
    translations/$${APP_NAME}-nl.ts \
    translations/$${APP_NAME}-nl_BE.ts \
    translations/$${APP_NAME}-pl.ts \
    translations/$${APP_NAME}-pt_BR.ts \
    translations/$${APP_NAME}-ru.ts \
    translations/$${APP_NAME}-sk.ts \
    translations/$${APP_NAME}-sv.ts

# packaging
scout_silica {
    DISTFILES += \
        rpm/$${TARGET}.spec
}

scout_silica {
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

    translations_files.files = $${TRANSLATIONS_FILES}
    translations_files.path = $${PREFIX}/share/$${APP_NAME}/translations
    INSTALLS += translations_files
}

scout_uuitk|scout_lomiri {
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
