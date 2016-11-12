#ifndef CONNECTION_H
#define CONNECTION_H

#include "microhttpserver.h"
#include <microhttpd.h>

#include <QByteArray>

namespace MicroHTTP {

class Server;

class Connection
{
public:
    enum State { Wait, Done, Error, NoInstance };

public:
    typedef char* keytype;

public:
    Connection(Server *server, MHD_Connection *connection);

    Server* server() { return m_server; }
    QByteArray& data() { return m_data; }
    MHD_Connection* connection() { return m_connection; }
    State state() { return m_state; }

    void setState(State s) { m_state = s; }
    void setData(QByteArray &data) { m_data = data; }

protected:
    Server *m_server = NULL;
    QByteArray m_data;
    MHD_Connection *m_connection = NULL;
    State m_state = Wait;
};

}

#endif // CONNECTION_H
