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
    static const keytype empty;

  public:
    Connection(Server *server, MHD_Connection *connection);

    Server* server() { return m_server; }
    QByteArray& data() { return m_data; }
    MHD_Connection* connection() { return m_connection; }
    State state() { return m_state; }
    QByteArray& postData() { return m_post_data; }

    void setState(State s) { m_state = s; }
    void setData(QByteArray &data) { m_data = data; }
    bool appendPostData(const char *upload_data, size_t upload_data_size);

  protected:
    Server *m_server = NULL;
    QByteArray m_data;
    QByteArray m_post_data;
    MHD_Connection *m_connection = NULL;
    State m_state = Wait;
  };

}

#endif // CONNECTION_H
