#ifndef CONNECTIONSTORE_H
#define CONNECTIONSTORE_H

#include "microhttpserver.h"
#include "microhttpconnection.h"

#include <microhttpd.h>

namespace MicroHTTP {

class Server;

class ConnectionStore
{
public:
    static Connection::keytype next(Server *server, MHD_Connection *connection);
    static void destroy(Connection::keytype);

    static Connection::State state(Connection::keytype key, Server* &server, MHD_Connection* &connection);

    static void setData(Connection::keytype key, QByteArray &data, bool error);

    static bool dataToSend(Connection::keytype key, QByteArray &data);
    static void serverDone(Connection::keytype key);

protected:
    ConnectionStore();

};

}

#endif // CONNECTIONSTORE_H
