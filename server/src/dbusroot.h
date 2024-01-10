/*
 * Copyright (C) 2019 Rinigus https://github.com/rinigus, 2019 Purism SPC
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

#ifndef DBUSROOT_H
#define DBUSROOT_H

#include "config.h"

#include <QDBusMessage>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>

/// \brief Interface with DBus: root methods
class DBusRoot : public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_ROOT)
  Q_PROPERTY(QString Url READ url)
  Q_PROPERTY(QString Version READ version)
  Q_PROPERTY(QString VersionLibs READ versionLibs)

public slots:
  void Activate(QMap<QString, QVariant> platform_data);
  void Open(QStringList uris, QMap<QString, QVariant> platform_data);
  void ActivateAction(QString action_name, QVariantList parameter, QMap<QString, QVariant> platform_data);

  void Connect(const QDBusMessage &message);
  void Quit();

public:

  DBusRoot(QString host, int port, QObject *parent = nullptr);

  QString url() const { return m_url; }
  QString version() const;
  QString versionLibs() const;

protected:
  QString m_url;
  QSet<QString> m_clients;
};

#endif // DBUSROOT_H
