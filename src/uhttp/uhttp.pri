INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/microhttpserver.cpp \
    $$PWD/microhttpconnectionstore.cpp \
    $$PWD/microhttpconnection.cpp \
    $$PWD/microhttpservicebase.cpp

HEADERS += \
    $$PWD/microhttpserver.h \
    $$PWD/microhttpconnectionstore.h \
    $$PWD/microhttpconnection.h \
    $$PWD/microhttpservicebase.h

LIBS += -lmicrohttpd
