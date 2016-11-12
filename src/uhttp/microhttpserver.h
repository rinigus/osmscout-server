#ifndef MICROHTTPSERVER_H
#define MICROHTTPSERVER_H

#include <microhttpd.h>
#include <mutex>
#include <set>

#include <QObject> // to provide timers access

namespace MicroHTTP {

class ServiceBase;

class Server: public QObject
{
    Q_OBJECT

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
    virtual void timerEvent(QTimerEvent *event); ///< Calls cleanup() internally

protected:
    std::mutex m_mutex;

    struct MHD_Daemon *m_daemon;
    ServiceBase *m_service;
    bool m_state = true;

    std::set<MHD_Connection*> m_connections_sleeping;
};

}

#endif // MICROHTTPSERVER_H
