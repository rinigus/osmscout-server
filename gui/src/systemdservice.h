/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2016-2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SYSTEMDSERVICE_H
#define SYSTEMDSERVICE_H

#include "config-common.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.osm.scout.server1.systemd_service
 */
class SystemDService: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return DBUS_INTERFACE_SYSTEMD; }

public:
    SystemDService(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~SystemDService();

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    inline bool enabled() const
    { return qvariant_cast< bool >(property("enabled")); }
    inline void setEnabled(bool value)
    { setProperty("enabled", QVariant::fromValue(value)); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void enabledChanged();
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::SystemDService systemd_service;
      }
    }
  }
}
#endif
