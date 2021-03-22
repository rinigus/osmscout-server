/*
 * Copyright (C) 2016-2021 Rinigus https://github.com/rinigus
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

#include "config-common.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

#include <iostream>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QSettings>

bool activate_server_tcp()
{
  QSettings settings;
  QNetworkAccessManager nam;
  QNetworkRequest request;

  QString host = settings.value(HTTP_SERVER_SETTINGS "host").toString();
  QString port = settings.value(HTTP_SERVER_SETTINGS "port").toString();
  QUrl url = QStringLiteral("http://%1:%2/v1/activate").arg(host).arg(port);

  request.setUrl(url);

  QEventLoop loop;
  loop.connect(&nam, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
  QNetworkReply *reply = nam.get(request);
  loop.exec();

  bool res = (reply->isFinished() && reply->error() == QNetworkReply::NoError);
  reply->deleteLater();
  return res;
}


bool port_free(int port)
{
  int sockfd;
  struct sockaddr_in serv_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      std::cerr << "ERROR opening socket\n";
      return false;
    }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  bool port_free = false;
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    port_free = true;

  close(sockfd);

  return port_free;
}


bool wait_till_port_is_free(int port)
{
  const size_t cycles = 10;
  for (size_t i=0; i < cycles; ++i)
    if (port_free(port))
      {
        return true;
      }
    else
      {
        std::cout << "Waiting for port: " << port << std::endl;
        sleep(1);
      }

  return false;
}
