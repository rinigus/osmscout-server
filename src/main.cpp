/*
  Copyright (C) 2016 rinigus <rinigus.git@gmail.com>
  License: LGPL
*/

#include "appsettings.h"
#include "config.h"

#ifdef IS_CONSOLE_QT
#include <QGuiApplication>
#define APP_PREFIX ""
#endif

#ifdef IS_SAILFISH_OS
#include <sailfishapp.h>
#include <QtQuick>
#define APP_PREFIX "harbour-"
#endif

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

    app->setApplicationName(APP_PREFIX "osmscout-server");
    app->setOrganizationName(APP_PREFIX "osmscout-server");

    AppSettings settings;
    settings.initDefaults();

#ifdef IS_SAILFISH_OS
    QScopedPointer<QQuickView> v(SailfishApp::createView());
    QQmlContext *rootContext = v->rootContext();

    rootContext->setContextProperty("programName", "OSMScout Server");
    rootContext->setContextProperty("programVersion", APP_VERSION);
    rootContext->setContextProperty("settingsOsmPrefix", OSM_SETTINGS);

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

    v->setSource(SailfishApp::pathTo("qml/osmscout-server.qml"));
    v->show();

#endif

    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      osmScoutMaster, &DBMaster::onSettingsChanged );

    return app->exec();
}
