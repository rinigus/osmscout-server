/*
  Copyright (C) 2016 rinigus <rinigus.git@gmail.com>
  License: LGPL
*/

#ifdef IS_CONSOLE_QT
#include <QGuiApplication>
#endif

#ifdef IS_SAILFISH_OS
#include <sailfishapp.h>
#include <QtQuick>
#endif

#include "appsettings.h"

// QtWebApp headers
#include "httplistener.h"
#include "requestmapper.h"

// LIB OSM Scout interface
#include "dbmaster.h"

#include <iostream>

DBMaster *osmScoutMaster = NULL;

int main(int argc, char *argv[])
{    
#ifdef IS_CONSOLE_QT
    QScopedPointer<QGuiApplication> app(new QGuiApplication(argc,argv));
#endif

#ifdef IS_SAILFISH_OS
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
#endif

    app->setApplicationName("osmscout-server");
    app->setOrganizationName("osmscout-server");

    AppSettings settings;
    settings.initDefaults();

#ifdef IS_SAILFISH_OS
    QScopedPointer<QQuickView> v(SailfishApp::createView());
    QQmlContext *rootContext = v->rootContext();

    rootContext->setContextProperty("programName", "SystemDataScope");
    rootContext->setContextProperty("programVersion", APP_VERSION);
    rootContext->setContextProperty("settings", &settings);
#endif

    // setup OSM Scout
    osmScoutMaster = new DBMaster();

    if (osmScoutMaster == nullptr)
    {
        std::cerr << "Failed to allocate DBMaster" << std::endl;
        return -1;
    }

    settings.beginGroup("http-listener");
    new HttpListener(&settings,new RequestMapper(app.data()),app.data());
    settings.endGroup();

#ifdef IS_SAILFISH_OS

    v->setSource(SailfishApp::pathTo("qml/main.qml"));
    v->show();

#endif
    return app->exec();
}
