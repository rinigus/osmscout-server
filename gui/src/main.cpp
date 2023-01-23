/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifdef IS_QTCONTROLS_QT
#include <QApplication>
#endif

#ifdef IS_SAILFISH_OS
#include <sailfishapp.h>
#endif // of IS_SAILFISH_OS

#include <QtQuick>
#include <QtQml>
#include <QQmlApplicationEngine>

#include "config-common.h"
#include "filemodel.h"
#include "servercontroller.h"
#include "trackdbusservice.h"

#include "appsettings.h"
#include "geomaster.h"
#include "mapmanager.h"
#include "infohub.h"
#include "logger.h"
#include "modulechecker.h"
#include "serverdbusroot.h"
#include "systemdservice.h"

#include <QDBusConnection>
#include <QCommandLineParser>
#include <QTranslator>
#ifdef IS_QTCONTROLS_QT
#include <QQuickStyle>
#endif

#include <QDebug>

#include <iostream>

////////////////////////////////////////////////

int main(int argc, char *argv[])
{
#ifdef IS_QTCONTROLS_QT
#ifdef DEFAULT_FALLBACK_STYLE
  if (QQuickStyle::name().isEmpty())
    QQuickStyle::setStyle(DEFAULT_FALLBACK_STYLE);
#endif
#endif

#ifdef IS_SAILFISH_OS
  QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
#endif
#ifdef IS_QTCONTROLS_QT
  QScopedPointer<QApplication> app(new QApplication(argc,argv));
#endif
#if defined(IS_SAILFISH_OS) || defined(IS_QTCONTROLS_QT)
  qmlRegisterType<FileModel>("harbour.osmscout.server.FileManager", 1, 0, "FileModel");
#endif

  app->setApplicationName(APP_NAME);
  app->setOrganizationName(APP_NAME);
#ifdef IS_QTCONTROLS_QT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  app->setDesktopFileName(APP_NAME ".desktop");
#endif
#endif
  app->setApplicationVersion(APP_VERSION);

  {
    QString tr_path(TRANSLATION_FOLDER);

    if ( !tr_path.isEmpty() )
      {
        QString locale = QLocale::system().name();
        QTranslator *translator = new QTranslator();

        if ( !translator->load(QLocale(),
                       #ifdef IS_QTCONTROLS_QT
                               "osmscout-server",
                       #else
                               APP_NAME,
                       #endif
                               "-",
                               tr_path) )
          qWarning() << "Failed to load translation for " << locale
                     << " " << tr_path;

        app->installTranslator(translator);
      }
  }

  // set fallback icons for platforms that need it
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
#ifdef IS_QTCONTROLS_QT
  QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/icons/fallback");
#endif
#endif

  // deal with command line options
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::translate("main", "OSM Scout Server GUI"));
  parser.addHelpOption();
  parser.addVersionOption();

  // Process the actual command line arguments given by the user
  parser.process(*app);

  // establish d-bus connection
  QDBusConnection dbusconnection = QDBusConnection::sessionBus();

  // setup proxies
  AppSettings settings(DBUS_SERVICE, DBUS_PATH_SETTINGS, dbusconnection);
  GeoMaster geoMaster(DBUS_SERVICE, DBUS_PATH_GEOMASTER, dbusconnection);
  InfoHub infoHub(DBUS_SERVICE, DBUS_PATH_INFOHUB, dbusconnection);
  Logger logger(DBUS_SERVICE, DBUS_PATH_LOGGER, dbusconnection);
  SystemDService systemd_service(DBUS_SERVICE, DBUS_PATH_SYSTEMD, dbusconnection);
  MapManager manager(DBUS_SERVICE, DBUS_PATH_MANAGER, dbusconnection);
  ModuleChecker modules(DBUS_SERVICE, DBUS_PATH_MODULES, dbusconnection);
  ServerDBusRoot serverDBusRoot(DBUS_SERVICE, DBUS_PATH_ROOT, dbusconnection);

  ServerController serverController;
  TrackDBusService service;

  // reconnect to server if it appears and reload all properties
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &serverController, &ServerController::onServiceAppeared);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &serverDBusRoot, &ServerDBusRoot::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &settings, &AppSettings::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &geoMaster, &GeoMaster::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &infoHub, &InfoHub::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &logger, &Logger::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &systemd_service, &SystemDService::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &manager, &MapManager::reloadData);
  QObject::connect(&service, &TrackDBusService::serviceAppeared,
                   &modules, &ModuleChecker::reloadData);

  // ////////////////////////////
  // QML setup

  // disable new QML connection syntax debug messages for as long as
  // older Qt versions (5.12 and older) are supported
  QLoggingCategory::setFilterRules(QStringLiteral("qt.qml.connections=false"));

#ifdef IS_SAILFISH_OS
  QScopedPointer<QQuickView> v;
  v.reset(SailfishApp::createView());
  QQmlContext *rootContext = v->rootContext();
#endif
#ifdef IS_QTCONTROLS_QT
  QQmlApplicationEngine engine;
  QQmlContext *rootContext = engine.rootContext();
#endif

  if (rootContext)
    {
      rootContext->setContextProperty("programName", "OSM Scout Server");
      rootContext->setContextProperty("programVersion", APP_VERSION);
      rootContext->setContextProperty("settingsMapManagerPrefix", MAPMANAGER_SETTINGS);
      rootContext->setContextProperty("settingsGeneralPrefix", GENERAL_SETTINGS);
      rootContext->setContextProperty("settingsOsmPrefix", OSM_SETTINGS);
      rootContext->setContextProperty("settingsSpeedPrefix", ROUTING_SPEED_SETTINGS);
      rootContext->setContextProperty("settingsGeomasterPrefix", GEOMASTER_SETTINGS);
      rootContext->setContextProperty("settingsMapnikPrefix", MAPNIKMASTER_SETTINGS);
      rootContext->setContextProperty("settingsValhallaPrefix", VALHALLA_MASTER_SETTINGS);
      rootContext->setContextProperty("settingsRequestMapperPrefix", REQUEST_MAPPER_SETTINGS);

      rootContext->setContextProperty("serverController", &serverController);
      rootContext->setContextProperty("service", &service);

      rootContext->setContextProperty("serverDBusRoot", &serverDBusRoot);
      rootContext->setContextProperty("settings", &settings);
      rootContext->setContextProperty("infohub", &infoHub);
      rootContext->setContextProperty("logger", &logger);
      rootContext->setContextProperty("manager", &manager);
      rootContext->setContextProperty("modules", &modules);
      rootContext->setContextProperty("systemd_service", &systemd_service);
      rootContext->setContextProperty("geocoder", &geoMaster);

#if defined(IS_SAILFISH_OS)
      // hack to make main menu consistent with expectations
      // at Sailfish OS.
      rootContext->setContextProperty("reverseMainMenu", true);
#else
      rootContext->setContextProperty("reverseMainMenu", false);
#endif
    }

#ifdef IS_SAILFISH_OS
  if (v)
    {
      v->setSource(SailfishApp::pathTo("qml/osmscout-server.qml"));
      v->show();
    }
#endif
#ifdef IS_QTCONTROLS_QT
  engine.load(QUrl(QStringLiteral("qrc:/qml/osmscout-server.qml")));
#endif

  return app->exec();
}
