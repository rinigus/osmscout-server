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

#include "microhttpconnection.h"

const MicroHTTP::Connection::keytype MicroHTTP::Connection::empty = 0;

MicroHTTP::Connection::Connection(MicroHTTP::Server *server, MHD_Connection *connection):
    m_server(server),
    m_connection(connection)
{

}

void MicroHTTP::Connection::appendPostData(const char *upload_data, size_t upload_data_size)
{
  m_post_data.append(upload_data, upload_data_size);
}
