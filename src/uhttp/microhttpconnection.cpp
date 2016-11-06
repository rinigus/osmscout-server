#include "microhttpconnection.h"

MicroHTTP::Connection::Connection(MicroHTTP::Server *server, MHD_Connection *connection):
    m_server(server),
    m_connection(connection)
{

}
