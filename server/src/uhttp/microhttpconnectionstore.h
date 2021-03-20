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

    static bool appendPostData(Connection::keytype key, const char *upload_data, size_t upload_data_size);
    static QByteArray getPostData(Connection::keytype key);

    static void setData(Connection::keytype key, QByteArray &data, bool error);

    static bool dataToSend(Connection::keytype key, QByteArray &data);
    static void serverDone(Connection::keytype key);

protected:
    ConnectionStore();

};

}

#endif // CONNECTIONSTORE_H
