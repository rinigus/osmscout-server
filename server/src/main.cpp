/*
 * Copyright (C) 2016-2021 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "appsettings.h"
#include "config.h"

#if defined(USE_OSMSCOUT) && defined(USE_OSMSCOUT_MAP_QT)
#include <QGuiApplication>
#else
#include <QCoreApplication>
#endif

#ifdef IS_SAILFISH_OS
#include <sailfishapp.h>
#endif // of IS_SAILFISH_OS

#include "consolelogger.h"
#include "rollinglogger.h"

// HTTP server
#include "microhttpserver.h"
#include "requestmapper.h"

// DBus interface
#include "dbusroot.h"
#include "infohubdbusadaptor.h"
#include "mapmanagerdbusadaptor.h"
#include "valhallamapmatcherdbus.h"
#include "valhallamapmatcherdbusadaptor.h"

#include "dbmaster.h"
#include "geomaster.h"
#include "mapboxglmaster.h"
#include "mapnikmaster.h"
#include "valhallamaster.h"

#include "mapmanager.h"
#include "infohub.h"
#include "modulechecker.h"

#include "systemdservice.h"
#include "util.hpp"

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTranslator>

#include <QDebug>

#include <iostream>
#include <csignal>

#ifdef USE_CURL
#include <curl/curl.h>
#endif

////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  bool has_logger_console = true;

#ifdef USE_CURL
  if ( curl_global_init(CURL_GLOBAL_DEFAULT ) )
    {
      std::cerr << "Error initializing libcurl\n";
      return -10;
    }
#endif

#if defined(USE_OSMSCOUT) && defined(USE_OSMSCOUT_MAP_QT)
  QScopedPointer<QGuiApplication> app(new QGuiApplication(argc,argv));
#else
  QScopedPointer<QCoreApplication> app(new QCoreApplication(argc,argv));
#endif

  app->setApplicationName(APP_NAME);
  app->setOrganizationName(APP_NAME);
  app->setApplicationVersion(APP_VERSION);

  {
    QString tr_path;

#if defined(IS_UUITK)
    tr_path = "./translations";
#elif defined(IS_QTCONTROLS_QT)
    tr_path = ":/i18n";
#elif defined(IS_SAILFISH_OS)
    tr_path = SailfishApp::pathTo(QString("translations")).toLocalFile();
#endif

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

  // deal with command line options
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::translate("main", "OSM Scout Server"));
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption optionQuiet(QStringList() << "quiet",
                                 QCoreApplication::translate("main", "Do not output logs when running in console mode"));
  parser.addOption(optionQuiet);

#ifdef USE_SYSTEMD
  QCommandLineOption optionSystemD(QStringList() << "systemd",
                                   QCoreApplication::translate("main", "Run the server in SystemD socket-activated mode"));
  parser.addOption(optionSystemD);
#endif

  QCommandLineOption optionDBusActivated(QStringList() << "dbus-activated",
                                         QCoreApplication::translate("main", "Run the server in DBus activated mode"));
  parser.addOption(optionDBusActivated);

  QCommandLineOption optionConsole(QStringList() << "console",
                                   "Deprecated, not used anymore. Kept for compatibility with old systemd .service files");
  parser.addOption(optionConsole);

    // Process the actual command line arguments given by the user
  parser.process(*app);

  // check logger related options
  has_logger_console = !parser.isSet(optionQuiet);

  // setup loggers
  ConsoleLogger *console_logger = nullptr;
  if (has_logger_console)
    console_logger = new ConsoleLogger(app.data());

  // this logger always available for GUI
  RollingLogger rolling_logger(app.data());

  // can use after the app name is defined
  AppSettings settings;
  settings.initDefaults();

  InfoHub::instance()->onSettingsChanged();

  // establish d-bus connection
  QDBusConnection dbusconnection = QDBusConnection::sessionBus();

  // enable systemd interaction
  SystemDService systemd_service;

//  // Close other instance if it was started by systemd or DBus activation
//  bool wait_for_port = false;

//  if (!parser.isSet(optionDBusActivated) &&
//      dbusconnection.isConnected())
//    {
//      // Handling of DBus activation
//      auto pid = dbusconnection.interface()->servicePid(DBUS_SERVICE);
//      if (pid.isValid())
//        {
//          std::cout << "DBus service already registered by process " << pid.value() << ".\n";
//          std::cout << "Sending close signal.\n";
//          kill(pid.value(), SIGUSR1);
//          wait_for_port = true;
//        }
//    }

//#ifdef USE_SYSTEMD
//  // stop systemD service and socket if running as a separate application
//  if (!parser.isSet(optionSystemD))
//    {
//      systemd_service.stop();
//      wait_for_port = true;
//    }
//#endif

//  // wait till the used ports are freed. here, the timeout is used internally in
//  // the used wait function
//  if (wait_for_port)
//    {
//      int http_port = settings.valueInt(HTTP_SERVER_SETTINGS "port");

//      if (!wait_till_port_is_free(http_port))
//        {
//          std::cerr << "Port " << http_port << " is occupied\n";
//          return -1;
//        }
//    }

  // check installed modules
  ModuleChecker modules;

  // setup Map Manager
  MapManager::Manager manager(app.data());

  // init logger
  rolling_logger.onSettingsChanged();

  // setup Geocoder-NLP
  if (GeoMaster::instance() == nullptr)
    {
      std::cerr << "Failed to allocate GeoMaster" << std::endl;
      return -2;
    }

  // setup Mapbox GL
  if (MapboxGLMaster::instance() == nullptr)
    {
      std::cerr << "Failed to allocate MapboxGLMaster" << std::endl;
      return -3;
    }

#ifdef USE_MAPNIK
  if (MapnikMaster::instance() == nullptr)
    {
      std::cerr << "Failed to allocate MapnikMaster" << std::endl;
      return -4;
    }
#endif

#ifdef USE_VALHALLA
  if (ValhallaMaster::instance() == nullptr)
    {
      std::cerr << "Failed to allocate ValhallaMaster" << std::endl;
      return -5;
    }
#endif

#ifdef USE_OSMSCOUT
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    DBMaster::instance(), &DBMaster::onSettingsChanged );
#endif
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    GeoMaster::instance(), &GeoMaster::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    MapboxGLMaster::instance(), &MapboxGLMaster::onSettingsChanged );
#ifdef USE_MAPNIK
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    MapnikMaster::instance(), &MapnikMaster::onSettingsChanged );
#endif
#ifdef USE_VALHALLA
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    ValhallaMaster::instance(), &ValhallaMaster::onSettingsChanged );
#endif
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    InfoHub::instance(), &InfoHub::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &manager, &MapManager::Manager::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &modules, &ModuleChecker::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &rolling_logger, &RollingLogger::onSettingsChanged );

#ifdef USE_OSMSCOUT
  QObject::connect( &manager, &MapManager::Manager::databaseOsmScoutChanged,
                    DBMaster::instance(), &DBMaster::onDatabaseChanged );
#endif

  QObject::connect( &manager, &MapManager::Manager::databaseGeocoderNLPChanged,
                    GeoMaster::instance(), &GeoMaster::onGeocoderNLPChanged);
  QObject::connect( &manager, &MapManager::Manager::databasePostalChanged,
                    GeoMaster::instance(), &GeoMaster::onPostalChanged);
  QObject::connect( &manager, &MapManager::Manager::selectedMapChanged,
                    GeoMaster::instance(), &GeoMaster::onSelectedMapChanged);
  QObject::connect( &manager, &MapManager::Manager::databaseMapboxGLChanged,
                    MapboxGLMaster::instance(), &MapboxGLMaster::onMapboxGLChanged );
#ifdef USE_MAPNIK
  QObject::connect( &manager, &MapManager::Manager::databaseMapnikChanged,
                    MapnikMaster::instance(), &MapnikMaster::onMapnikChanged );
#endif
#ifdef USE_VALHALLA
  QObject::connect( &manager, &MapManager::Manager::databaseValhallaChanged,
                    ValhallaMaster::instance(), &ValhallaMaster::onValhallaChanged );
#endif

  if (console_logger)
    QObject::connect( &manager, &MapManager::Manager::errorMessage,
                      console_logger, &ConsoleLogger::onErrorMessage);

  // all is connected, load map manager settings
  manager.onSettingsChanged();

  // register singlar handler
  signal(SIGTERM, [](int /*sig*/){ qApp->quit(); });
  signal(SIGINT, [](int /*sig*/){ qApp->quit(); });
  signal(SIGHUP, [](int /*sig*/){ qApp->quit(); });

  // quit application if receiving SIGUSR1 and was DBus activated
  if ( parser.isSet(optionDBusActivated) )
    signal(SIGUSR1, [](int /*sig*/){ qApp->quit(); });
  else
    signal(SIGUSR1, [](int /*sig*/){ std::cout << "Ignoring SIGUSR1" << std::flush; });

  int return_code = 0;

#ifdef USE_VALHALLA
  ValhallaMaster::instance()->start();
#endif

  // prepare server by processing all outstanding events
  // that way, it will be ready immediately to process requests
  app->processEvents();

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
        return -100;
      }

    // connect request mapper to the settings
    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      &requests, &RequestMapper::onSettingsChanged );

    // enable idle timeout shutdown if started by systemd or DBus activation
    if ( parser.isSet(optionDBusActivated)
    #ifdef USE_SYSTEMD
        || parser.isSet(optionSystemD)
    #endif
        )
      {
        QObject::connect(InfoHub::instance(), &InfoHub::activitySig,
                         &requests, &RequestMapper::updateLastCall,
                         Qt::QueuedConnection);
        QObject::connect(&requests, &RequestMapper::idleTimeout,
                         app.data(), QCoreApplication::quit );
      }

    // add d-bus interface
#ifdef USE_VALHALLA
    ValhallaMapMatcherDBus valhallaMapMatcherDBus;
    new ValhallaMapMatcherDBusAdaptor(&valhallaMapMatcherDBus);
    if (!dbusconnection.registerObject(DBUS_PATH_MAPMATCHING, &valhallaMapMatcherDBus))
      InfoHub::logWarning(
            QCoreApplication::translate("main",
                                        "Failed to register DBus object: %1").arg(DBUS_PATH_MAPMATCHING));
    else
      dbusconnection.connect(QString(), "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged",
                             &valhallaMapMatcherDBus, SLOT(onNameOwnerChanged(QString,QString,QString)));
    valhallaMapMatcherDBus.activate();
#endif

    new InfoHubDBusAdaptor(InfoHub::instance());
    if (!dbusconnection.registerObject(DBUS_PATH_INFOHUB, InfoHub::instance()))
      InfoHub::logWarning(
            QCoreApplication::translate("main",
                                        "Failed to register DBus object: %1").arg(DBUS_PATH_INFOHUB));

    new MapManager::ManagerDBusAdaptor(&manager);
    if (!dbusconnection.registerObject(DBUS_PATH_MANAGER, &manager))
      InfoHub::logWarning(
            QCoreApplication::translate("main",
                                        "Failed to register DBus object: %1").arg(DBUS_PATH_MANAGER));

#define DBUSREG(path, objptr, prop) \
  if (!dbusconnection.registerObject(path, objptr, prop )) \
     InfoHub::logWarning( \
             QCoreApplication::translate("main", \
             "Failed to register DBus object: %1").arg(path));

    DBUSREG(DBUS_PATH_GEOMASTER, GeoMaster::instance(),
            QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals);

    DBUSREG(DBUS_PATH_LOGGER, &rolling_logger,
            QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals);

    DBUSREG(DBUS_PATH_MODULES, &modules,
            QDBusConnection::ExportAllProperties);

    DBUSREG(DBUS_PATH_SETTINGS, &settings,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties |
            QDBusConnection::ExportAllSignals);

    DBUSREG(DBUS_PATH_SYSTEMD, &systemd_service,
            QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals);

    DBusRoot dbusRoot(host, port);
    DBUSREG(DBUS_PATH_ROOT, &dbusRoot,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties);

    // register dbus service
    if (!dbusconnection.registerService(DBUS_SERVICE))
      InfoHub::logWarning(
            QCoreApplication::translate("main",
                                        "Failed to register DBus service: %1").arg(DBUS_SERVICE));

    return_code = app->exec();
  }

#ifdef USE_SYSTEMD
  // if the service is enabled, start it after we leave the server
  if (!parser.isSet(optionSystemD) && systemd_service.enabled())
    systemd_service.start();
#endif

  return return_code;
}
