TARGET = harbour-osmscout-server-module-fonts

CONFIG += sailfishapp_qml sailfishapp_i18n
SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

qml.files = qml/*.qml
qml.path = /usr/share/harbour-osmscout-server-module-fonts/qml

desktop.files = harbour-osmscout-server-module-fonts.desktop
desktop.path = /usr/share/applications

INSTALLS = qml desktop

fonts.files = fonts
fonts.path = /usr/share/$${TARGET}
INSTALLS += fonts

OTHER_FILES += \
    rpm/harbour-osmscout-server-module-fonts.spec \
    qml/harbour-osmscout-server-module-fonts.qml \
    qml/harbour-osmscout-server-module-fonts.qml \
    qml/Cover.qml \
    qml/MainPage.qml \
    harbour-osmscout-server-module-fonts.desktop \
    harbour-osmscout-server-module-fonts.png
