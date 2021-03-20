/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
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

#ifndef MICROHTTPSERVER_H
#define MICROHTTPSERVER_H

#include <microhttpd.h>
#include <mutex>
#include <set>

//#define HAS_MICRO_HTTP_CLEANUP_TIMER

#ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
#include <QObject> // to provide timers access
#endif

#if MHD_VERSION > 0x00097000
#define MICROHTTP_Result MHD_Result
#else
#define MICROHTTP_Result int
#endif


namespace MicroHTTP {

class ServiceBase;

class Server
        #ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
        : public QObject
        #endif
{
#ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
    Q_OBJECT
#endif

public:

    //////////////////////////////////////
    /// \brief Server constructor
    /// \param service object providing service to connections
    /// \param port server listening on the port
    /// \param address server binding to this interface given in x.x.x.x form. If NULL, server binds to all interfaces
    /// \param systemd set to true if systemd socket is supposed to be used
    ///
    explicit Server(ServiceBase *service,
                    unsigned int port,
                    const char *address, bool systemd = false);
    virtual ~Server();

    operator bool() const { return m_state; }

    ServiceBase* service() const { return m_service; }

    void suspend(MHD_Connection *conn); ///< Puts connection to sleep
    void resume(MHD_Connection *conn);  ///< Resumes connection
    void cleanup();                     ///< Call periodically to check on available connections

protected:
#ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
    virtual void timerEvent(QTimerEvent *event); ///< Calls cleanup() internally
#endif

protected:
    std::mutex m_mutex;

    struct MHD_Daemon *m_daemon;
    ServiceBase *m_service;
    bool m_state = true;

    std::set<MHD_Connection*> m_connections_sleeping;
};

}

#endif // MICROHTTPSERVER_H
