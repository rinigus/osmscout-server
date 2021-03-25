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

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "microhttpservicebase.h"

#include <QThreadPool>
#include <QTimer>
#include <QObject>
#include <QString>

#include <time.h>

/**
  The request mapper dispatches incoming HTTP requests to controller classes
  depending on the requested path.
*/

class RequestMapper : public QObject, public MicroHTTP::ServiceBase
{
  Q_OBJECT

public:

  explicit RequestMapper(QObject *parent = 0);
  virtual ~RequestMapper();

  /**
      Dispatch incoming HTTP requests to different controllers depending on the URL.
    */
  virtual unsigned int service(const char *url, MHD_Connection *, MHD_Response *,
                               MicroHTTP::Connection::keytype connection_id);
  virtual void loguri(const char *uri);

public slots:
  void onSettingsChanged();

protected:
  QThreadPool m_pool;

  // availibility of different backends
  bool m_available_geocodernlp{false};
  bool m_available_mapboxgl{false};
  bool m_available_mapnik{false};
  bool m_available_osmscout{false};
  bool m_available_valhalla{false};

  QString m_info_enable_backends;
};

#endif // REQUESTMAPPER_H
