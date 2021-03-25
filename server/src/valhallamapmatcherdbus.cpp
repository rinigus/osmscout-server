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

#include "valhallamapmatcherdbus.h"

#include "dbustracker.h"
#include "infohub.h"

#include <QDebug>

ValhallaMapMatcherDBus::ValhallaMapMatcherDBus(QObject *parent):
  QObject(parent)
{
  connect(DBusTracker::instance(), &DBusTracker::serviceDisappeared,
          this, &ValhallaMapMatcherDBus::onServiceDisappeared);
}

ValhallaMapMatcherDBus::~ValhallaMapMatcherDBus()
{
}

void ValhallaMapMatcherDBus::activate()
{
  emit activeChanged(true);
}

QString ValhallaMapMatcherDBus::update(int mode, double lat, double lon, double accuracy, const QDBusMessage &message)
{
  const QString caller = message.service();
  QSharedPointer<ValhallaMapMatcher> &c = m_matchers[caller][mode];
  if (c.isNull())
    {
      c.reset(new ValhallaMapMatcher(mode));
      if ( c.isNull() ||
          !c->start() )
        return "{\"error\": \"Failed to allocate or start Map Matcher\"}";

      DBusTracker::instance()->track(caller);
    }
  return c->update(lat, lon, accuracy);
}

bool ValhallaMapMatcherDBus::reset(int mode, const QDBusMessage &message)
{
  const QString caller = message.service();
  QSharedPointer<ValhallaMapMatcher> c = m_matchers[caller][mode];
  return !c.isNull() && c->start();
}

bool ValhallaMapMatcherDBus::stop(int mode, const QDBusMessage &message)
{
  const QString caller = message.service();
  if (m_matchers[caller].contains(mode))
    m_matchers[caller].remove(mode);
  return true;
}

bool ValhallaMapMatcherDBus::stop(const QDBusMessage &message)
{
  const QString caller = message.service();
  if (m_matchers.contains(caller))
    m_matchers.remove(caller);
  return true;
}

void ValhallaMapMatcherDBus::onServiceDisappeared(QString name)
{
  if (m_matchers.contains(name))
    {
      InfoHub::logInfo(tr("Closing map matching service for DBus client %1").arg(name));
      m_matchers.remove(name);
    }
}

#endif
