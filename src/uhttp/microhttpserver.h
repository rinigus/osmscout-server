#ifndef MICROHTTPSERVER_H
#define MICROHTTPSERVER_H

#include <microhttpd.h>
#include <mutex>
#include <set>

//#define HAS_MICRO_HTTP_CLEANUP_TIMER

#ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
#include <QObject> // to provide timers access
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
    /// \param parent
    ///
    explicit Server(ServiceBase *service,
                    unsigned int port,
                    const char *address);
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
