/*
  Copyright (C) 2016 rinigus <rinigus.git@gmail.com>
  License: LGPL
*/

#include "appsettings.h"
#include "config.h"

#ifdef IS_CONSOLE_QT

#ifdef USE_OSMSCOUT_MAP_CAIRO
#include <QCoreApplication>
#endif
#ifdef USE_OSMSCOUT_MAP_QT
#include <QGuiApplication>
#endif

#define APP_PREFIX ""

#endif // of IS_CONSOLE_QT

#include "consolelogger.h"

#ifdef IS_SAILFISH_OS
#include <sailfishapp.h>
#include <QtQuick>
#define APP_PREFIX "harbour-"

#include "rollinglogger.h"
#endif // of IS_SAILFISH_OS

// HTTP server
#include "microhttpserver.h"
#include "requestmapper.h"

// LIB OSM Scout interface
#include "dbmaster.h"

#include "infohub.h"

#include <iostream>

DBMaster *osmScoutMaster = NULL;

// global variable keeping the Hub
extern InfoHub infoHub;


int main(int argc, char *argv[])
{
#ifdef IS_CONSOLE_QT
#ifdef USE_OSMSCOUT_MAP_CAIRO
    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc,argv));
#endif
#ifdef USE_OSMSCOUT_MAP_QT
    QScopedPointer<QGuiApplication> app(new QGuiApplication(argc,argv));
#endif
#endif

#ifdef IS_SAILFISH_OS
    RollingLogger rolling_logger;

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
#endif

    app->setApplicationName(APP_PREFIX "osmscout-server");
    app->setOrganizationName(APP_PREFIX "osmscout-server");

    // can use after the app name is defined
    AppSettings settings;
    settings.initDefaults();

    infoHub.onSettingsChanged();

#ifdef IS_CONSOLE_QT
    ConsoleLogger _logger;
#endif

#ifdef IS_SAILFISH_OS
    //ConsoleLogger _logger_console;

    rolling_logger.onSettingsChanged();

    QScopedPointer<QQuickView> v(SailfishApp::createView());
    QQmlContext *rootContext = v->rootContext();

    rootContext->setContextProperty("programName", "OSM Scout Server");
    rootContext->setContextProperty("programVersion", APP_VERSION);
    rootContext->setContextProperty("settingsOsmPrefix", OSM_SETTINGS);
    rootContext->setContextProperty("settingsSpeedPrefix", ROUTING_SPEED_SETTINGS);

    rootContext->setContextProperty("settings", &settings);
    rootContext->setContextProperty("infohub", &infoHub);
    rootContext->setContextProperty("logger", &rolling_logger);
#endif

    // setup OSM Scout
    osmScoutMaster = new DBMaster();

    if (osmScoutMaster == nullptr)
    {
        std::cerr << "Failed to allocate DBMaster" << std::endl;
        return -1;
    }

    // setup HTTP server
    settings.beginGroup("http-listener");
    int port = settings.valueInt("port");
    QString host = settings.valueString("host");
    settings.endGroup();

    RequestMapper requests;
    MicroHTTP::Server http_server( &requests, port, host.toStdString().c_str() );

    if ( !http_server )
    {
        std::cerr << "Failed to start HTTP server" << std::endl;
        return -2;
    }

#ifdef IS_SAILFISH_OS

    v->setSource(SailfishApp::pathTo("qml/osmscout-server.qml"));
    v->show();

#endif

    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      osmScoutMaster, &DBMaster::onSettingsChanged );
    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      &infoHub, &InfoHub::onSettingsChanged );
#ifdef IS_SAILFISH_OS
    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      &rolling_logger, &RollingLogger::onSettingsChanged );
#endif

    return app->exec();
}
