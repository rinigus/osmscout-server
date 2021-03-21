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
    translations/$${APP_NAME}-cs.ts \
    translations/$${APP_NAME}-de.ts \
    translations/$${APP_NAME}-es.ts \
    translations/$${APP_NAME}-et.ts \
    translations/$${APP_NAME}-fi.ts \
    translations/$${APP_NAME}-fr.ts \
    translations/$${APP_NAME}-nb.ts \
    translations/$${APP_NAME}-nl.ts \
    translations/$${APP_NAME}-pl.ts \
    translations/$${APP_NAME}-ru.ts \
    translations/$${APP_NAME}-sv.ts \
    translations/$${APP_NAME}-nl_BE.ts \
    translations/$${APP_NAME}-it_IT.ts \
    translations/$${APP_NAME}-pt_BR.ts

# packaging
scout_silica {
    DISTFILES += \
        rpm/$${TARGET}.spec
}

