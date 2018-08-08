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

#include "microhttpconnectionstore.h"
#include "microhttpconnection.h"

#include <QMutexLocker>
#include <QHash>

using namespace MicroHTTP;

// static objects to keep the store data
static Connection::keytype next_key = 0;
static QMutex mutex;
static QHash<Connection::keytype, Connection> store_data;

using namespace MicroHTTP;

ConnectionStore::ConnectionStore()
{

}

Connection::keytype ConnectionStore::next(Server *server, MHD_Connection *connection)
{
    QMutexLocker lk(&mutex);
    next_key += 1;
    store_data.insert(next_key, Connection(server, connection));
    return next_key;
}

void ConnectionStore::destroy(Connection::keytype k)
{
    QMutexLocker lk(&mutex);
    store_data.remove(k);
}

Connection::State ConnectionStore::state(Connection::keytype key, Server *&server, MHD_Connection *&connection)
{
    QMutexLocker lk(&mutex);
    auto iter = store_data.find(key);
    if (iter == store_data.end())
    {
        server = NULL;
        connection = NULL;
        return Connection::NoInstance;
    }

    server = iter->server();
    connection = iter->connection();
    return iter->state();
}

void ConnectionStore::setData(Connection::keytype key, QByteArray &data, bool error)
{
    QMutexLocker lk(&mutex);
    auto iter = store_data.find(key);
    if (iter == store_data.end())
        return;

    iter->setData(data);
    if (error)
        iter->setState(Connection::Error);
    else
        iter->setState(Connection::Done);

    iter->server()->resume( iter->connection() );
}

bool ConnectionStore::dataToSend(Connection::keytype key, QByteArray &data)
{
    QMutexLocker lk(&mutex);
    auto iter = store_data.find(key);
    if ( iter == store_data.end() ||
         (iter->state()!=Connection::Done && iter->state()!=Connection::Error) )
        return false;

    data = iter->data();
    return true;
}

bool ConnectionStore::appendPostData(Connection::keytype key, const char *upload_data, size_t upload_data_size)
{
  QMutexLocker lk(&mutex);
  auto iter = store_data.find(key);
  if (iter == store_data.end())
      return false;

  return iter->appendPostData(upload_data, upload_data_size);
}

QByteArray ConnectionStore::getPostData(Connection::keytype key)
{
  QMutexLocker lk(&mutex);
  auto iter = store_data.find(key);
  if (iter == store_data.end())
      return QByteArray();

  return iter->postData();
}

void ConnectionStore::serverDone(Connection::keytype key)
{
    QMutexLocker lk(&mutex);
    auto iter = store_data.find(key);
    if (iter == store_data.end()) return;

    store_data.remove(key);
}
