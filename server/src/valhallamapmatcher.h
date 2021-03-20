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

#ifndef VALHALLAMAPMATCHER_H
#define VALHALLAMAPMATCHER_H

#ifdef USE_VALHALLA

#include <QObject>

#include <QByteArray>
#include <QGeoCoordinate>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>

class ValhallaMapMatcher : public QObject
{
  Q_OBJECT

public:
  enum Mode { Auto=1, AutoShorter=2, Bicycle=3, Bus=4, Pedestrian=5, Unknown=-1 };

public:
  explicit ValhallaMapMatcher(int mode);
  virtual ~ValhallaMapMatcher();

  bool start();
  bool stop();
  QString update(double lat, double lon, double accuracy);

signals:
  void activity();

protected:

  // helper class to keep earlier recorded map
  // matching results. while setting the value,
  // `set` method will return true if the new
  // value of a property is different from the
  // recorded one
  struct Properties {
    QHash<QString,int> m_property_int;
    QHash<QString,double> m_property_double;
    QHash<QString,QString> m_property_string;

    bool set(const QString &key, int value);
    bool set(const QString &key, double value);
    bool set(const QString &key, const QString &value);
  };

  struct Point {
    QGeoCoordinate coordinate;
    double accuracy;

    Point() {}
    Point(const QGeoCoordinate &c, double a): coordinate(c), accuracy(a) {}
  };

  static QString mode2str(Mode mode);
  static Mode int2mode(int i);

protected:

  void preFillRequest();
  void fillRequest(const QJsonArray &shape, double accuracy, QByteArray &request);

  void setProperty(const QString &key, int value, QJsonObject &response);
  void setProperty(const QString &key, double value, QJsonObject &response);
  void setProperty(const QString &key, const QString &value, QJsonObject &response);

protected:
  QList<Point> m_locations;
  Point m_last_position_info;

  Mode m_mode{Unknown};
  Properties m_properties;
  QJsonObject m_request_base;
};


#endif // USE_VALHALLA
#endif // VALHALLAMAPMATCHER_H
