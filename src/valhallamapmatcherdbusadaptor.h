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

#ifndef VALHALLAMAPMATCHERDBUSADAPTOR_H
#define VALHALLAMAPMATCHERDBUSADAPTOR_H

#include "valhallamapmatcherdbus.h"
#include "config.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>

class ValhallaMapMatcherDBusAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_MAPMATCHING)
  Q_PROPERTY(bool Active READ active NOTIFY ActiveChanged)

public:
  ValhallaMapMatcherDBusAdaptor(ValhallaMapMatcherDBus *parent);

  bool active() const { return true; }

signals:
  void ActiveChanged(bool active);

public slots:
  QString Update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message);

  bool Reset(int mode, const QDBusMessage &message);
  bool Stop(int mode, const QDBusMessage &message);
  bool Stop(const QDBusMessage &message);

private:
  ValhallaMapMatcherDBus *m;
};

#endif // VALHALLAMAPMATCHERDBUSADAPTOR_H

#endif // USE_VALHALLA
