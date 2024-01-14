/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2023 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SERVERDBUSROOT_H
#define SERVERDBUSROOT_H

#include "config-common.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class ServerDBusRoot : public QDBusAbstractInterface
{
  Q_OBJECT

public:
  static inline const char *staticInterfaceName()
  { return DBUS_INTERFACE_ROOT; }

public:
  ServerDBusRoot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

  ~ServerDBusRoot();

  Q_PROPERTY(QString Version READ version NOTIFY reloadData)

  Q_PROPERTY(QString VersionLibs READ versionLibs NOTIFY reloadData)

  inline QString version() const
  { return qvariant_cast< QString >(property("Version")); }

  inline QString versionLibs() const
  { return qvariant_cast< QString >(property("VersionLibs")); }

public Q_SLOTS:
  inline void quit()
  {
    QList<QVariant> argumentList;
    asyncCallWithArgumentList(QStringLiteral("Quit"), argumentList);
  }

Q_SIGNALS:
  void reloadData();
};

#endif // SERVERDBUSROOT_H
