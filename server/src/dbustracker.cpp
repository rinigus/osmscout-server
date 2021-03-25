/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "dbustracker.h"

#include <QDBusConnection>

#include <QDebug>

DBusTracker* DBusTracker::s_instance = nullptr;

DBusTracker::DBusTracker(QObject *parent) : QObject(parent)
{
  QDBusConnection dbusconnection = QDBusConnection::sessionBus();
  dbusconnection.connect(QString(),
                         "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged",
                         this, SLOT(onNameOwnerChanged(QString,QString,QString)));
}

DBusTracker* DBusTracker::instance()
{
  if (!s_instance) s_instance = new DBusTracker();
  return s_instance;
}

int DBusTracker::numberOfServices()
{
  QMutexLocker lk(&m_mutex);
  return m_tracked.size();
}

void DBusTracker::track(const QString &service)
{
  QMutexLocker lk(&m_mutex);
  m_tracked.insert(service);
  qDebug() << "Tracking " << service;
}

void DBusTracker::onNameOwnerChanged(QString name, QString /*old_owner*/, QString new_owner)
{
  QMutexLocker lk(&m_mutex);
  if (new_owner.length() < 1 && m_tracked.contains(name))
    {
      m_tracked.remove(name);
      qDebug() << "Service disappeared " << name;
      emit serviceDisappeared(name);
    }
}
