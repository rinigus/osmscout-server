TEMPLATE = subdirs
SUBDIRS = server gui

include(common.pri)

# installs
styles.files = styles
styles.path = $$PREFIX/share/$${TARGET}
INSTALLS += styles

geocoder_data.files = data/geocoder-npl-tag-aliases.json
geocoder_data.path = $${PREFIX}/share/$${TARGET}/data
INSTALLS += geocoder_data

valhalla_data.extra = install -v -m 644 $$PWD/data/valhalla.json-$${VALHALLA_VERSION} \
   ${INSTALL_ROOT}/$${PREFIX}/share/$${TARGET}/data/valhalla.json ;
valhalla_data.path = $${PREFIX}/share/$${TARGET}/data
INSTALLS += valhalla_data

OTHER_FILES += rpm/osmscout-server.spec

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

scout_silica {
    DISTFILES += \
        rpm/$${TARGET}.spec
}

