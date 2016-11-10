#ifndef MICROHTTPSERVER_H
#define MICROHTTPSERVER_H

#include <microhttpd.h>

#include <QObject>
#include <QSet>
#include <QMutex>

namespace MicroHTTP {

class ServiceBase;

class Server : public QObject
{
    Q_OBJECT

signals:

public slots:

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
                    const char *address,
                    QObject *parent = 0);
    virtual ~Server();

    operator bool() const { return m_state; }

    ServiceBase* service() const { return m_service; }

    void suspend(MHD_Connection *conn); ///< Puts connection to sleep
    void resume(MHD_Connection *conn);  ///< Resumes connection

protected:
    virtual void timerEvent(QTimerEvent *event);

protected:
    QMutex m_mutex;

    struct MHD_Daemon *m_daemon;
    ServiceBase *m_service;
    bool m_state = true;

    QSet<MHD_Connection*> m_connections_sleeping;
};

}

#endif // MICROHTTPSERVER_H
