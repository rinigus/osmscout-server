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

#endif // of IS_CONSOLE_QT

#include "consolelogger.h"

#ifdef IS_SAILFISH_OS
#include <sailfishapp.h>
#include <QtQuick>
#include <QtQml>

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
#include "modulechecker.h"

#include "systemdservice.h"
#include "util.hpp"

#include <QTranslator>
#include <QCommandLineParser>

#include <QDebug>

#include <iostream>
#include <csignal>

#ifdef USE_CURL
#include <curl/curl.h>
#endif

// this is needed for connection with signals. Otherwise, access via static members
extern InfoHub infoHub;

////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  bool has_logger_console = false;

#ifdef IS_SAILFISH_OS
  bool has_logger_rolling = true;
#endif

#ifdef IS_CONSOLE_QT
  has_logger_console = true;
#endif

#ifdef USE_CURL
  if ( curl_global_init(CURL_GLOBAL_DEFAULT ) )
    {
      std::cerr << "Error initializing libcurl\n";
      return -10;
    }
#endif

#ifdef IS_CONSOLE_QT
#ifdef USE_OSMSCOUT_MAP_CAIRO
  QScopedPointer<QCoreApplication> app(new QCoreApplication(argc,argv));
#endif
#ifdef USE_OSMSCOUT_MAP_QT
  QScopedPointer<QGuiApplication> app(new QGuiApplication(argc,argv));
#endif
#endif

#ifdef IS_SAILFISH_OS
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

  QCommandLineOption optionConsole(QStringList() << "console",
                                   QCoreApplication::translate("main", "Run the server without GUI as a console application"));
  parser.addOption(optionConsole);

  QCommandLineOption optionQuiet(QStringList() << "quiet",
                                 QCoreApplication::translate("main", "Do not output logs when running in console mode"));
  parser.addOption(optionQuiet);

#ifdef USE_SYSTEMD
  QCommandLineOption optionSystemD(QStringList() << "systemd",
                                   QCoreApplication::translate("main", "Run the server in SystemD socket-activated mode"));
  parser.addOption(optionSystemD);
#endif

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

  // check logger related options
#ifdef IS_SAILFISH_OS
  if (parser.isSet(optionConsole)) // have to enable logger when running as GUI
    has_logger_rolling = false;

  if (parser.isSet(optionConsole))
    has_logger_console = !parser.isSet(optionQuiet);
#endif
#ifdef IS_CONSOLE_QT
  has_logger_console = !parser.isSet(optionQuiet);
#endif

  // setup loggers
  ConsoleLogger *console_logger = nullptr;
  if (has_logger_console)
    console_logger = new ConsoleLogger(app.data());

#ifdef IS_SAILFISH_OS
  RollingLogger *rolling_logger = nullptr;
  if (has_logger_rolling)
    rolling_logger = new RollingLogger(app.data());
#endif

  // can use after the app name is defined
  AppSettings settings;
  settings.initDefaults();

  infoHub.onSettingsChanged();

#ifdef USE_SYSTEMD
  // enable systemd interaction
  SystemDService systemd_service;

  // stop systemD service and socket if running as a separate application
  if (!parser.isSet(optionSystemD))
    systemd_service.stop();

  // wait till the used ports are freed. here, the timeout is used internally in
  // the used wait function
  if (!parser.isSet(optionSystemD))
    {
      int http_port = settings.valueInt(HTTP_SERVER_SETTINGS "port");
      int valhalla_port = settings.valueInt(VALHALLA_MASTER_SETTINGS "route_port");

      if (!wait_till_port_is_free(http_port))
        std::cerr << "Port " << http_port << " is occupied\n";

      if (!wait_till_port_is_free(valhalla_port))
        std::cerr << "Port " << valhalla_port << " is occupied\n";
    }
#endif

  // check installed modules
  ModuleChecker modules;

  // setup Map Manager
  MapManager::Manager manager(app.data());

#ifdef IS_SAILFISH_OS
  if (rolling_logger) rolling_logger->onSettingsChanged();

  QScopedPointer<QQuickView> v;
  QQmlContext *rootContext = nullptr;
  if (!parser.isSet(optionConsole))
    {
      v.reset(SailfishApp::createView());
      rootContext = v->rootContext();
    }

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

      rootContext->setContextProperty("settings", &settings);
      rootContext->setContextProperty("infohub", &infoHub);
      if (rolling_logger) rootContext->setContextProperty("logger", rolling_logger);
      rootContext->setContextProperty("manager", &manager);
      rootContext->setContextProperty("modules", &modules);
#ifdef USE_SYSTEMD
      rootContext->setContextProperty("systemd_service", &systemd_service);
#endif
    }
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
#ifdef IS_SAILFISH_OS
  if (rootContext) rootContext->setContextProperty("geocoder", geoMaster);
#endif

#ifdef USE_MAPNIK
  // setup Mapnik
  mapnikMaster = new MapnikMaster();

  if (mapnikMaster == nullptr)
    {
      std::cerr << "Failed to allocate MapnikMaster" << std::endl;
      return -3;
    }
#endif

#ifdef USE_VALHALLA
  // setup for Valhalla
  valhallaMaster = new ValhallaMaster(app.data());

  if (valhallaMaster == nullptr)
    {
      std::cerr << "Failed to allocate ValhallaMaster" << std::endl;
      return -4;
    }
#endif

#ifdef IS_SAILFISH_OS  
  if (v)
    {
      v->setSource(SailfishApp::pathTo("qml/osmscout-server.qml"));
      v->show();
    }
#endif

  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    osmScoutMaster, &DBMaster::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    geoMaster, &GeoMaster::onSettingsChanged );
#ifdef USE_MAPNIK
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    mapnikMaster, &MapnikMaster::onSettingsChanged );
#endif
#ifdef USE_VALHALLA
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    valhallaMaster, &ValhallaMaster::onSettingsChanged );
#endif
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
  QObject::connect( &manager, &MapManager::Manager::selectedMapChanged,
                    geoMaster, &GeoMaster::onSelectedMapChanged);

#ifdef USE_MAPNIK
  QObject::connect( &manager, &MapManager::Manager::databaseMapnikChanged,
                    mapnikMaster, &MapnikMaster::onMapnikChanged );
#endif
#ifdef USE_VALHALLA
  QObject::connect( &manager, &MapManager::Manager::databaseValhallaChanged,
                    valhallaMaster, &ValhallaMaster::onValhallaChanged );
#endif

  if (console_logger)
    QObject::connect( &manager, &MapManager::Manager::errorMessage,
                      console_logger, &ConsoleLogger::onErrorMessage);

#ifdef IS_SAILFISH_OS
  if (rolling_logger)
    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      rolling_logger, &RollingLogger::onSettingsChanged );
#endif

  // all is connected, load map manager settings
  manager.onSettingsChanged();

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

  // register singlar handler
  signal(SIGTERM, [](int /*sig*/){ qApp->quit(); });
  signal(SIGINT, [](int /*sig*/){ qApp->quit(); });
  signal(SIGHUP, [](int /*sig*/){ qApp->quit(); });

  int return_code = 0;

  {
    // setup HTTP server
    int port = settings.valueInt(HTTP_SERVER_SETTINGS "port");
    QString host = settings.valueString(HTTP_SERVER_SETTINGS "host");

    // start HTTP server
    RequestMapper requests;
    MicroHTTP::Server http_server( &requests, port, host.toStdString().c_str(),
#ifdef USE_SYSTEMD
                                   parser.isSet(optionSystemD)
#else
                                   false
#endif
                                   );

    if ( !http_server )
      {
        std::cerr << "Failed to start HTTP server" << std::endl;
        return -2;
      }

    // connect request mapper to the settings
    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      &requests, &RequestMapper::onSettingsChanged );

    // enable idle timeout shutdown if started by systemd
#ifdef USE_SYSTEMD
    if (parser.isSet(optionSystemD))
      QObject::connect(&requests, &RequestMapper::idleTimeout,
                       app.data(), QCoreApplication::quit );
#endif

    return_code = app->exec();
  }

#ifdef USE_SYSTEMD
  // if the service is enabled, start it after we leave the server
  if (!parser.isSet(optionSystemD) && systemd_service.enabled())
    systemd_service.start();
#endif

  return return_code;
}
