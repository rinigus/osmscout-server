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
#include <QtQml>
#define APP_PREFIX "harbour-"

#include "rollinglogger.h"
#include "filemodel.h"
#endif // of IS_SAILFISH_OS

// HTTP server
#include "microhttpserver.h"
#include "requestmapper.h"

// LIB OSM Scout interface
#include "dbmaster.h"

// Geocoder-NLP interface
#include "geomaster.h"

#include "mapmanager.h"
#include "infohub.h"

#include <QTranslator>
#include <QCommandLineParser>

#include <QDebug>

#include <iostream>

// this is needed for connection with signals. Otherwise, access via static members
extern InfoHub infoHub;

////////////////////////////////////////////////

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
  qmlRegisterType<FileModel>("harbour.osmscout.server.FileManager", 1, 0, "FileModel");
#endif

  app->setApplicationName(APP_PREFIX "osmscout-server");
  app->setOrganizationName(APP_PREFIX "osmscout-server");
  app->setApplicationVersion(APP_VERSION);

  {
    QString tr_path;

#ifdef IS_SAILFISH_OS
    tr_path = SailfishApp::pathTo(QString("translations")).toLocalFile();
#endif
    if ( !tr_path.isEmpty() )
      {
        QString locale = QLocale::system().name();
        QTranslator *translator = new QTranslator();

        if ( !translator->load(QLocale(), APP_PREFIX "osmscout-server", "-",
                               tr_path) )
          qWarning() << "Failed to load translation for " << locale
                     << " " << tr_path;

        app->installTranslator(translator);
      }
  }

  // deal with command line options
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::translate("main", "OSM Scout Server"));
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption optionDownload(QStringList() << "d" << "download",
                                    QCoreApplication::translate("main", "Start download of the maps"));
  parser.addOption(optionDownload);

  QCommandLineOption optionUpdate(QStringList() << "u" << "update",
                                  QCoreApplication::translate("main", "Update list of available maps"));
  parser.addOption(optionUpdate);

  QCommandLineOption optionListAvailable("list-available",
                                         QCoreApplication::translate("main", "List maps available on device"));
  parser.addOption(optionListAvailable);

  QCommandLineOption optionListSubscribed("list-subscribed",
                                          QCoreApplication::translate("main", "List subscribed maps"));
  parser.addOption(optionListSubscribed);

  QCommandLineOption optionListProvided("list-provided",
                                          QCoreApplication::translate("main", "List maps provided for download"));
  parser.addOption(optionListProvided);

  QCommandLineOption optionListMissing("list-missing",
                                          QCoreApplication::translate("main", "List missing maps"));
  parser.addOption(optionListMissing);

  QCommandLineOption optionSubscribe("sub",
                                     QCoreApplication::translate("main", "Subscribe to a <country> dataset"),
                                     QCoreApplication::translate("main", "country-id"));
  parser.addOption(optionSubscribe);

  QCommandLineOption optionUnSubscribe("unsub",
                                     QCoreApplication::translate("main", "Unsubscribe <country> dataset"),
                                     QCoreApplication::translate("main", "country-id"));
  parser.addOption(optionUnSubscribe);

  // Process the actual command line arguments given by the user
  parser.process(*app);

  // can use after the app name is defined
  AppSettings settings;
  settings.initDefaults();

  infoHub.onSettingsChanged();

  // setup Map Manager
  MapManager::Manager manager;

#ifdef IS_CONSOLE_QT
  ConsoleLogger console_logger;
#endif

#ifdef IS_SAILFISH_OS
  //ConsoleLogger _logger_console;

  rolling_logger.onSettingsChanged();

  QScopedPointer<QQuickView> v(SailfishApp::createView());
  QQmlContext *rootContext = v->rootContext();

  rootContext->setContextProperty("programName", "OSM Scout Server");
  rootContext->setContextProperty("programVersion", APP_VERSION);
  rootContext->setContextProperty("settingsMapManagerPrefix", MAPMANAGER_SETTINGS);
  rootContext->setContextProperty("settingsOsmPrefix", OSM_SETTINGS);
  rootContext->setContextProperty("settingsSpeedPrefix", ROUTING_SPEED_SETTINGS);
  rootContext->setContextProperty("settingsGeomasterPrefix", GEOMASTER_SETTINGS);
  rootContext->setContextProperty("settingsMapnikPrefix", MAPNIKMASTER_SETTINGS);

  rootContext->setContextProperty("settings", &settings);
  rootContext->setContextProperty("infohub", &infoHub);
  rootContext->setContextProperty("logger", &rolling_logger);
  rootContext->setContextProperty("manager", &manager);
#endif

  // setup OSM Scout
  osmScoutMaster = new DBMaster();

  if (osmScoutMaster == nullptr)
    {
      std::cerr << "Failed to allocate DBMaster" << std::endl;
      return -1;
    }

  // setup Geocoder-NLP
  geoMaster = new GeoMaster();

  if (geoMaster == nullptr)
    {
      std::cerr << "Failed to allocate GeoMaster" << std::endl;
      return -2;
    }

  // setup Mapnik
  mapnikMaster = new MapnikMaster();

  if (mapnikMaster == nullptr)
    {
      std::cerr << "Failed to allocate MapnikMaster" << std::endl;
      return -3;
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
                    geoMaster, &GeoMaster::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    mapnikMaster, &MapnikMaster::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &infoHub, &InfoHub::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &manager, &MapManager::Manager::onSettingsChanged );

  QObject::connect( &manager, &MapManager::Manager::databaseOsmScoutChanged,
                    osmScoutMaster, &DBMaster::onDatabaseChanged );

  QObject::connect( &manager, &MapManager::Manager::databaseGeocoderNLPChanged,
                    geoMaster, &GeoMaster::onGeocoderNLPChanged);
  QObject::connect( &manager, &MapManager::Manager::databasePostalChanged,
                    geoMaster, &GeoMaster::onPostalChanged);

  QObject::connect( &manager, &MapManager::Manager::databaseMapnikChanged,
                    mapnikMaster, &MapnikMaster::onMapnikChanged );

#ifdef IS_SAILFISH_OS
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &rolling_logger, &RollingLogger::onSettingsChanged );
#endif

#ifdef IS_CONSOLE_QT
  QObject::connect( &manager, &MapManager::Manager::errorMessage,
                    &console_logger, &ConsoleLogger::onErrorMessage);

#endif

  // all is connected, load map manager settings
  manager.onSettingsChanged();

#ifdef IS_CONSOLE_QT
  // check for sanity and perform the commands if requested
  if (!manager.storageAvailable())
    {
      std::cerr << "ERROR: The storage folder is not allocated or not configured\n";
      return -1;
    }

  if (parser.isSet(optionDownload))
    manager.getCountries();

  if (parser.isSet(optionUpdate))
    manager.updateProvided();

  if (parser.isSet(optionListAvailable))
    {
      std::cout << manager.getAvailableCountries().toStdString() << "\n";
      return 0;
    }

  if (parser.isSet(optionListSubscribed))
    {
      std::cout << manager.getRequestedCountries().toStdString() << "\n";
      return 0;
    }

  if (parser.isSet(optionListProvided))
    {
      std::cout << manager.getProvidedCountries().toStdString() << "\n";
      return 0;
    }

  if (parser.isSet(optionListMissing))
    {
      std::cout << manager.missingInfo().toStdString() << "\n";
      return 0;
    }

  if (!parser.value(optionSubscribe).isEmpty())
    {
      QString c = parser.value(optionSubscribe);
      std::cout << "Subscribing to " << c.toStdString() << "\n";
      manager.addCountry(c);
      return 0;
    }

  if (!parser.value(optionUnSubscribe).isEmpty())
    {
      QString c = parser.value(optionUnSubscribe);
      std::cout << "Unsubscribing from " << c.toStdString() << "\n";
      manager.rmCountry(c);
      return 0;
    }

#endif

  return app->exec();
}
