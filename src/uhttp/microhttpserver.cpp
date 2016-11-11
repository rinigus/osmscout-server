#include "microhttpserver.h"
#include "microhttpconnectionstore.h"
#include "microhttpservicebase.h"

#include <QString>
#include <QDebug>
#include <QFile>
#include <QMutexLocker>

#include <algorithm>
#include <functional>

#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

///////////////////////////////////////////////////////////////////////////////////
/// Helper functions

static void content_reader_free_callback(void *cls)
{
    MicroHTTP::Connection::keytype key = (MicroHTTP::Connection::keytype)cls;
    MicroHTTP::ConnectionStore::serverDone(key);

#ifdef DEBUG_CONNECTIONS
    qDebug() << "Finished: " << (size_t)key;
#endif
}

static ssize_t content_reader_callback (void *cls, uint64_t pos, char *buf, size_t max)
{
    MicroHTTP::Connection::keytype key = (MicroHTTP::Connection::keytype)cls;
    MicroHTTP::Server *server;
    MHD_Connection *connection;

    MicroHTTP::Connection::State state = MicroHTTP::ConnectionStore::state(key, server, connection);

#ifdef DEBUG_CONNECTIONS
    qDebug() << "Content reader: " << (size_t)key << " " << state;
#endif

    if (server == NULL || state == MicroHTTP::Connection::NoInstance || !(*server))
        return MHD_CONTENT_READER_END_WITH_ERROR;

    if ( state == MicroHTTP::Connection::Wait )
    {
        server->suspend(connection);
        return 0;
    }

    QByteArray data;
    if ( !MicroHTTP::ConnectionStore::dataToSend(key, data) )
        return MHD_CONTENT_READER_END_WITH_ERROR;

    if (pos >= (size_t)data.size())
        return MHD_CONTENT_READER_END_OF_STREAM;

    size_t p = (size_t)pos;
    size_t tosend = std::min(max, data.size() - p);

    memcpy(buf, data.constData() + pos, tosend);

    return tosend;
}

static int answer_to_connection (void *cls, struct MHD_Connection *connection,
                                 const char *url, const char *method,
                                 const char */*version*/, const char */*upload_data*/,
                                 size_t */*upload_data_size*/, void **/*con_cls*/)
{
    //qDebug() << "answer:" << url << " / " << method << " / version " << version ;

    if (strcmp("GET", method))
    {
        //qDebug() << method << " -> not GET";
        return MHD_NO;
    }

    struct MHD_Response *response;
    MicroHTTP::Server *server = (MicroHTTP::Server*)cls;
    MicroHTTP::Connection::keytype
            connection_id = MicroHTTP::ConnectionStore::next(server, connection);

    int ret;

    response =
            MHD_create_response_from_callback(-1, 1024*1024,
                                              content_reader_callback,
                                              connection_id,
                                              content_reader_free_callback);

    unsigned int status_code =
            server->service()->service(url, connection, response, connection_id);

    ret = MHD_queue_response (connection, status_code, response);
    MHD_destroy_response (response);

#ifdef DEBUG_CONNECTIONS
    qDebug() << "Started: " << (size_t)connection_id;
#endif

    return ret;
}

void* uri_logger(void * cls, const char * uri, struct MHD_Connection */*con*/)
{
    MicroHTTP::Server *server = (MicroHTTP::Server*)cls;
    server->service()->loguri(uri);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// MicroHTTP::Server class
///

MicroHTTP::Server::Server(ServiceBase *service,
                          unsigned int port, const char *addrstring, QObject *parent) :
    QObject(parent),
    m_service(service)
{
    // Listen on specified address only
    struct sockaddr_in server_address;

    memset (&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (addrstring == NULL) server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    else
    {
        if ( inet_aton(addrstring, &server_address.sin_addr ) == 0 )
        {
            qDebug() << "Wrong interface address: " << addrstring;
            m_state = false;
            return;
        }
    }

    m_daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_SUSPEND_RESUME,
                                 port /*ignered since its in SOCK_ADDR OPTION*/,
                                 NULL, NULL,
                                 answer_to_connection, this,
                                 MHD_OPTION_SOCK_ADDR, &server_address,
                                 MHD_OPTION_CONNECTION_LIMIT, 100,
                                 MHD_OPTION_CONNECTION_TIMEOUT, 600, // seconds
                                 //MHD_OPTION_NOTIFY_COMPLETED, request_completed, this,
                                 MHD_OPTION_URI_LOG_CALLBACK, uri_logger, this,
                                 MHD_OPTION_END);
    if (m_daemon == NULL)
    {
        m_state = false;
        return;
    }

    startTimer(1000);
}

MicroHTTP::Server::~Server()
{
    QMutexLocker _lk(&m_mutex);

    m_state = false; // indicates that we are going to shutdown
    for (MHD_Connection *conn: m_connections_sleeping)
        MHD_resume_connection(conn);
    m_connections_sleeping.clear();

    MHD_stop_daemon (m_daemon);
}

void MicroHTTP::Server::timerEvent(QTimerEvent */*event*/)
{
    QMutexLocker _lk(&m_mutex);

    for (MHD_Connection *conn: m_connections_sleeping)
    {
        MHD_resume_connection(conn);
#ifdef DEBUG_CONNECTIONS
        qDebug() << "Resume by timer: " << (size_t)conn;
#endif
    }
    m_connections_sleeping.clear();
}

void MicroHTTP::Server::suspend(MHD_Connection *conn)
{
    QMutexLocker _lk(&m_mutex);

    if (!m_state) return;

    MHD_suspend_connection(conn);
    m_connections_sleeping.insert(conn);

#ifdef DEBUG_CONNECTIONS
    qDebug() << "Suspend: " << (size_t)conn;
#endif
}

void MicroHTTP::Server::resume(MHD_Connection *conn)
{
    QMutexLocker _lk(&m_mutex);

#ifdef DEBUG_CONNECTIONS
    qDebug() << "Called to resume: " << (size_t)conn;
#endif

    if (!m_state || !m_connections_sleeping.contains(conn)) return;

    MHD_resume_connection(conn);

#ifdef DEBUG_CONNECTIONS
    qDebug() << "Resumed: " << (size_t)conn;
#endif

    m_connections_sleeping.remove(conn);
}
