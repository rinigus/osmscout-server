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

#ifdef USE_VALHALLA

#ifndef VALHALLAMAPMATCHERDBUS_H
#define VALHALLAMAPMATCHERDBUS_H

#include "valhallamapmatcher.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QHash>
#include <QSharedPointer>

class ValhallaMapMatcherDBus : public QObject
{
  Q_OBJECT

public:
  ValhallaMapMatcherDBus(QObject *parent=nullptr);
  ~ValhallaMapMatcherDBus();

  void activate();

  void dbusServiceDisappeared(QString service);

public slots:
  //////////////////////////////////////////////////////////////////////////////////////////
  /// NB! when adding slots for DBus export, don't forget to add them in the adaptor as well

  QString update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message);

  bool reset(int mode, const QDBusMessage &message);
  bool stop(int mode, const QDBusMessage &message);
  bool stop(const QDBusMessage &message);

signals:
  void activeChanged(bool);

protected:
  QSharedPointer<ValhallaMapMatcher> get(int mode, const QString &caller);

private:
  /// \brief map matchers stored in a map
  ///
  /// For each client, a map matchers are organized according to
  /// the mode. This allows to have multiple modes per one client
  QHash< QString, QHash<int, QSharedPointer<ValhallaMapMatcher> > > m_matchers;
};

#endif // VALHALLAMAPMATCHERDBUS_H

#endif // USE_VALHALLA
