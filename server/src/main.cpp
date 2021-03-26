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
#include "dbustracker.h"
#include "idletracker.h"

#include "systemdservice.h"
#include "util.hpp"

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTranslator>

#include <QDebug>

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <iostream>
#include <csignal>

#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef USE_CURL
#include <curl/curl.h>
#endif

// use after socket_fd could have been allocated
#define CHECK_FOR_ERROR(cond, message, retval) \
  if (cond) { \
    std::cerr << message << std::endl; \
    if (socket_fd >= 0) close(socket_fd); \
    return retval; \
  }

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

  QCommandLineOption optionListen(QStringList() << "listen",
                                         QCoreApplication::translate("main",
                                                                     "Listen for connection on configured port and start full server on activity"));
  parser.addOption(optionListen);

  QCommandLineOption optionConsole(QStringList() << "console",
                                   "Deprecated, not used anymore. Kept for compatibility with old systemd .service files");
  parser.addOption(optionConsole);

  // Process the actual command line arguments given by the user
  parser.process(*app);

  // set start flags
  startedByDaemon = false;
  startedByDBus = parser.isSet(optionDBusActivated);
#ifdef USE_SYSTEMD
  // set to false by default for version compiled without systemd support
  startedBySystemD = parser.isSet(optionSystemD);
#endif

  // check sanity of options
  if (startedBySystemD && parser.isSet(optionListen))
    {
      std::cerr << "Error in specified options: cannot start with --listen and --systemd options"
                << std::endl;
      return -19;
    }

  // handle DBus activation first
  if (startedByDBus && activate_server_tcp())
    {
      // server was activated through TCP connection
      // closing this process as it's work is done
      return 0;
    }

  // can use after the app name is defined
  AppSettings settings;
  settings.initDefaults();

  // used to listen and, if in other modes, later in server creation
  struct sockaddr_in server_address;
  if (!fill_sockaddr(server_address))
    return 5;

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  /// Listen to activity on socket and fork full server when detected
  ///
  int socket_fd = -1;
  if (parser.isSet(optionListen))
    {
      socket_fd = socket(AF_INET, SOCK_STREAM, 0);
      CHECK_FOR_ERROR(socket_fd < 0,
                      "Cannot create socket file descriptor", 1);

      int on = 1;
      CHECK_FOR_ERROR(setsockopt(socket_fd, SOL_SOCKET,  SO_REUSEADDR,
                                 (char *)&on, sizeof(on)) < 0,
                      "Error: setsockopt()", 1);

      CHECK_FOR_ERROR(ioctl(socket_fd, FIONBIO, (char *)&on) < 0,
                      "Error: ioctl()", 1);

      CHECK_FOR_ERROR(bind(socket_fd, (struct sockaddr *)&server_address,
                           sizeof(server_address)) < 0,
                      "Error: bind()", 1);

      CHECK_FOR_ERROR(listen(socket_fd, 8) < 0, // 2nd argument: queue length for pending connections
                      "Error: listen()", 1);

      // poll loop for parent that is escaped by child
      pid_t main_pid = getpid();
      for (bool isparent = true; isparent; )
        {
          // init poll
          struct pollfd fds[1];
          nfds_t nfds = 1;

          memset(fds, 0 , sizeof(fds));
          fds[0].fd = socket_fd;
          fds[0].events = POLLIN;

          int pres = poll(fds, nfds, -1);

          CHECK_FOR_ERROR(pres <= 0,
                          "Error: poll() returned " << pres, 1);

          pid_t child = fork();
          CHECK_FOR_ERROR(child < 0,
                          "Error: fork()", -1);

          isparent = (child > 0);

          if (isparent)
            {
              int wstatus;
              wait(&wstatus);
              CHECK_FOR_ERROR( !WIFEXITED(wstatus),
                               "Error: child process terminated abnormally", -2);
              CHECK_FOR_ERROR( WEXITSTATUS(wstatus),
                               "Error: child process finished with error code " << WEXITSTATUS(wstatus), -3 );
            }
          else
            startedByDaemon = true;
        }

      // set to die if the parent dies
      // linux specific solution
      CHECK_FOR_ERROR(prctl(PR_SET_PDEATHSIG, SIGTERM) == -1,
                      "Error: prctl() in forked server", 10);

      CHECK_FOR_ERROR(getppid() != main_pid,
                      "Closing as parent died while starting the forked process", 0);

      std::cout << "Starting full server" << std::endl;
      startedByDaemon = true;
    }

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  /// Start of the full server
  ///

#ifdef USE_SYSTEMD
  // fill socket fd if started by systemd
  if (startedBySystemD)
    {
      if (sd_listen_fds(0) != 1)
        {
          std::cerr << "Number of SystemD-provided file descriptors is different from one"
                    << std::endl;
          return -2;
        }

      socket_fd = SD_LISTEN_FDS_START + 0;
    }
#endif

  // check logger related options
  has_logger_console = !parser.isSet(optionQuiet);

  // setup loggers
  ConsoleLogger *console_logger = nullptr;
  if (has_logger_console)
    console_logger = new ConsoleLogger(app.data());

  // this logger always available for GUI
  RollingLogger rolling_logger(app.data());

  InfoHub::instance()->onSettingsChanged();

  // check installed modules
  ModuleChecker modules;

  // setup Map Manager
  MapManager::Manager::instance()->setParent(app.data());

  // init logger
  rolling_logger.onSettingsChanged();

  // setup Geocoder-NLP
  CHECK_FOR_ERROR(GeoMaster::instance() == nullptr,
                  "Failed to allocate GeoMaster", 2);
  GeoMaster::instance()->setParent(app.data());

  // setup Mapbox GL
  CHECK_FOR_ERROR(MapboxGLMaster::instance() == nullptr,
                  "Failed to allocate MapboxGLMaster", 3);
  MapboxGLMaster::instance()->setParent(app.data());

#ifdef USE_MAPNIK
  CHECK_FOR_ERROR(MapnikMaster::instance() == nullptr,
                  "Failed to allocate MapnikMaster", 4);
  MapnikMaster::instance()->setParent(app.data());
#endif

#ifdef USE_VALHALLA
  CHECK_FOR_ERROR(ValhallaMaster::instance() == nullptr,
                  "Failed to allocate ValhallaMaster", 5);
  ValhallaMaster::instance()->setParent(app.data());
#endif

#ifdef USE_OSMSCOUT
  DBMaster::instance()->setParent(app.data());
#endif

  DBusTracker::instance()->setParent(app.data());
  InfoHub::instance()->setParent(app.data());
  SystemDService::instance()->setParent(app.data());

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
                    MapManager::Manager::instance(), &MapManager::Manager::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &modules, &ModuleChecker::onSettingsChanged );
  QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                    &rolling_logger, &RollingLogger::onSettingsChanged );

#ifdef USE_OSMSCOUT
  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::databaseOsmScoutChanged,
                    DBMaster::instance(), &DBMaster::onDatabaseChanged );
#endif

  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::databaseGeocoderNLPChanged,
                    GeoMaster::instance(), &GeoMaster::onGeocoderNLPChanged);
  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::databasePostalChanged,
                    GeoMaster::instance(), &GeoMaster::onPostalChanged);
  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::selectedMapChanged,
                    GeoMaster::instance(), &GeoMaster::onSelectedMapChanged);
  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::databaseMapboxGLChanged,
                    MapboxGLMaster::instance(), &MapboxGLMaster::onMapboxGLChanged );
#ifdef USE_MAPNIK
  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::databaseMapnikChanged,
                    MapnikMaster::instance(), &MapnikMaster::onMapnikChanged );
#endif
#ifdef USE_VALHALLA
  QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::databaseValhallaChanged,
                    ValhallaMaster::instance(), &ValhallaMaster::onValhallaChanged );
#endif

  if (console_logger)
    QObject::connect( MapManager::Manager::instance(), &MapManager::Manager::errorMessage,
                      console_logger, &ConsoleLogger::onErrorMessage);

  // all is connected, load map manager settings
  MapManager::Manager::instance()->onSettingsChanged();

  // register singlar handler
  signal(SIGTERM, [](int /*sig*/){ qApp->quit(); });
  signal(SIGINT, [](int /*sig*/){ qApp->quit(); });
  signal(SIGHUP, [](int /*sig*/){ qApp->quit(); });

  int return_code = 0;

#ifdef USE_VALHALLA
  ValhallaMaster::instance()->start();
#endif

  // prepare server by processing all outstanding events
  // that way, it will be ready immediately to process requests
  app->processEvents();

  {
    // start HTTP server
    RequestMapper requests;
    MicroHTTP::Server http_server( &requests, server_address, socket_fd );

    CHECK_FOR_ERROR( !http_server,
                     "Failed to start HTTP server", 100);

    // connect request mapper to the settings
    QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                      &requests, &RequestMapper::onSettingsChanged );

    // enable idle timeout shutdown if started by systemd or DBus activation
    if ( startedByDaemon || startedByDBus || startedBySystemD )
      {
        IdleTracker *idle = new IdleTracker( app.data() );

        QObject::connect(idle, &IdleTracker::idleTimeout,
                         app.data(), QCoreApplication::quit, Qt::QueuedConnection );

        QObject::connect( &settings, &AppSettings::osmScoutSettingsChanged,
                          idle, &IdleTracker::onSettingsChanged );
      }

    // establish d-bus connection
    QDBusConnection dbusconnection = QDBusConnection::sessionBus();

    // add d-bus interface
#ifdef USE_VALHALLA
    ValhallaMapMatcherDBus valhallaMapMatcherDBus;
    new ValhallaMapMatcherDBusAdaptor(&valhallaMapMatcherDBus);
    if (!dbusconnection.registerObject(DBUS_PATH_MAPMATCHING, &valhallaMapMatcherDBus))
      InfoHub::logWarning(
            QCoreApplication::translate("main",
                                        "Failed to register DBus object: %1").arg(DBUS_PATH_MAPMATCHING));
    valhallaMapMatcherDBus.activate();
#endif

    new InfoHubDBusAdaptor(InfoHub::instance());
    if (!dbusconnection.registerObject(DBUS_PATH_INFOHUB, InfoHub::instance()))
      InfoHub::logWarning(
            QCoreApplication::translate("main",
                                        "Failed to register DBus object: %1").arg(DBUS_PATH_INFOHUB));

    new MapManager::ManagerDBusAdaptor(MapManager::Manager::instance());
    if (!dbusconnection.registerObject(DBUS_PATH_MANAGER, MapManager::Manager::instance()))
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

    DBUSREG(DBUS_PATH_SYSTEMD, SystemDService::instance(),
            QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals);

    int port = settings.valueInt(HTTP_SERVER_SETTINGS "port");
    QString host = settings.valueString(HTTP_SERVER_SETTINGS "host");
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
  // only check systemd options if not started by alternative daemon
  if (!startedByDaemon)
    {
      // enable if user switched automatic activation on
      // in interactive of dbus activated session
      if (!startedBySystemD && SystemDService::instance()->enabled())
        SystemDService::instance()->start();
      // if the service is disabled and was started by systemd
      else if (startedBySystemD && !SystemDService::instance()->enabled())
        SystemDService::instance()->stop();
    }
#endif

  // close open socket_fd
  if (socket_fd >= 0) close(socket_fd);

  return return_code;
}
