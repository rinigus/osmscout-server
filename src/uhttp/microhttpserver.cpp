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

#include "microhttpserver.h"
#include "microhttpconnectionstore.h"
#include "microhttpservicebase.h"

#include <QByteArray>

#include <algorithm>
#include <functional>
#include <iostream>

#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

//#define DEBUG_CONNECTIONS

///////////////////////////////////////////////////////////////////////////////////
/// Helper functions

static void content_reader_free_callback(void *cls)
{
  MicroHTTP::Connection::keytype key = (MicroHTTP::Connection::keytype)cls;
  MicroHTTP::ConnectionStore::serverDone(key);

#ifdef DEBUG_CONNECTIONS
  std::cout << "Finished: " << (size_t)key << std::endl;
#endif
}

static ssize_t content_reader_callback (void *cls, uint64_t pos, char *buf, size_t max)
{
  MicroHTTP::Connection::keytype key = (MicroHTTP::Connection::keytype)cls;
  MicroHTTP::Server *server;
  MHD_Connection *connection;

  MicroHTTP::Connection::State state = MicroHTTP::ConnectionStore::state(key, server, connection);

#ifdef DEBUG_CONNECTIONS
  std::cout << "Content reader: " << (size_t)key << " " << state << std::endl;
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
                                 const char */*version*/, const char *upload_data,
                                 size_t *upload_data_size, void **con_cls)
{
#ifdef DEBUG_CONNECTIONS
  std::cout << "answer:" << url << " / " << method << std::endl;
#endif

  MicroHTTP::Connection::keytype connection_id = MicroHTTP::Connection::empty;
  MicroHTTP::Server *server = (MicroHTTP::Server*)cls;

  // dealing with POST
  if (strcmp("POST", method) == 0)
    {
      if (*con_cls == NULL) // new connection
        {
          connection_id = MicroHTTP::ConnectionStore::next(server, connection);
          *con_cls = connection_id;
          return MHD_YES;
        }

      connection_id = MicroHTTP::Connection::keytype(*con_cls);
      if (*upload_data_size != 0)
        {
          bool res = MicroHTTP::ConnectionStore::appendPostData(connection_id, upload_data, *upload_data_size);
          if (!res)
            {
              std::cerr << "Dropping POST request due to its large size: " << url << std::endl;
              MicroHTTP::ConnectionStore::serverDone(connection_id);
              return MHD_NO;
            }

          *upload_data_size = 0;
          return MHD_YES;
        }
    }

  // dealing with GET
  else if (strcmp("GET", method) == 0)
    {
      connection_id = MicroHTTP::ConnectionStore::next(server, connection);
    }

  // unsupported method
  else
    {
      return MHD_NO;
    }

  struct MHD_Response *response;

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
  std::cout << "Started: " << (size_t)connection_id << std::endl;
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
                          unsigned int port, const char *addrstring, bool systemd) :
  #ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
  QObject(),
  #endif
  m_service(service)
{
  // Listen on specified address only
#ifdef USE_SYSTEMD
  if (!systemd)
#endif
    { // regular application
      struct sockaddr_in server_address;

      memset (&server_address, 0, sizeof(server_address));
      server_address.sin_family = AF_INET;
      server_address.sin_port = htons(port);

      if (addrstring == NULL) server_address.sin_addr.s_addr = htonl(INADDR_ANY);
      else
        {
          if ( inet_aton(addrstring, &server_address.sin_addr ) == 0 )
            {
              std::cerr << "Wrong interface address: " << addrstring << std::endl;
              m_state = false;
              return;
            }
        }

      m_daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_SUSPEND_RESUME,
                                   port /*ignored since its in SOCK_ADDR OPTION*/,
                                   NULL, NULL,
                                   answer_to_connection, this,
                                   MHD_OPTION_SOCK_ADDR, &server_address,
                                   MHD_OPTION_CONNECTION_LIMIT, 100,
                                   MHD_OPTION_CONNECTION_TIMEOUT, 600, // seconds
                                   //MHD_OPTION_NOTIFY_COMPLETED, request_completed, this,
                                   MHD_OPTION_URI_LOG_CALLBACK, uri_logger, this,
                                   MHD_OPTION_END);
    }
#ifdef USE_SYSTEMD
  else
    { // systemd service
      if (sd_listen_fds(0) != 1)
        {
          std::cerr << "Number of SystemD-provided file descriptors is different from one\n";
        }
      else
        {
          int fd = SD_LISTEN_FDS_START + 0;

          m_daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_SUSPEND_RESUME | MHD_USE_ITC,
                                       port /*ignored since its in SOCK_ADDR OPTION*/,
                                       NULL, NULL,
                                       answer_to_connection, this,
                                       MHD_OPTION_LISTEN_SOCKET, fd,
                                       MHD_OPTION_CONNECTION_LIMIT, 100,
                                       MHD_OPTION_CONNECTION_TIMEOUT, 600, // seconds
                                       MHD_OPTION_URI_LOG_CALLBACK, uri_logger, this,
                                       MHD_OPTION_END);
        }
    }
#endif
  if (m_daemon == NULL)
    {
      m_state = false;
      return;
    }

#ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
  startTimer(15000);
#endif
}

MicroHTTP::Server::~Server()
{
  std::lock_guard<std::mutex> _lk(m_mutex);

  m_state = false; // indicates that we are going to shutdown
  for (MHD_Connection *conn: m_connections_sleeping)
    MHD_resume_connection(conn);
  m_connections_sleeping.clear();

  MHD_stop_daemon (m_daemon);
}

void MicroHTTP::Server::cleanup()
{
  std::lock_guard<std::mutex> _lk(m_mutex);

  if (!m_state) return;

  for (MHD_Connection *conn: m_connections_sleeping)
    {
      MHD_resume_connection(conn);
#ifdef DEBUG_CONNECTIONS
      std::cout << "Resume by timer: " << (size_t)conn << std::endl;
#endif
    }
  m_connections_sleeping.clear();
}


#ifdef HAS_MICRO_HTTP_CLEANUP_TIMER
void MicroHTTP::Server::timerEvent(QTimerEvent */*event*/)
{
  cleanup();
}
#endif


void MicroHTTP::Server::suspend(MHD_Connection *conn)
{
  std::lock_guard<std::mutex> _lk(m_mutex);

  if (!m_state) return;

  MHD_suspend_connection(conn);
  m_connections_sleeping.insert(conn);

#ifdef DEBUG_CONNECTIONS
  std::cout << "Suspend: " << (size_t)conn << std::endl;
#endif
}

void MicroHTTP::Server::resume(MHD_Connection *conn)
{
  std::lock_guard<std::mutex> _lk(m_mutex);


#ifdef DEBUG_CONNECTIONS
  std::cout << "Called to resume: " << (size_t)conn << std::endl;
#endif

  if (!m_state || m_connections_sleeping.count(conn) == 0) return;

  MHD_resume_connection(conn);

#ifdef DEBUG_CONNECTIONS
  std::cout << "Resumed: " << (size_t)conn << std::endl;
#endif

  m_connections_sleeping.erase(conn);
}
