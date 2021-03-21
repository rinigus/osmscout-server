/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GEOMASTER_H
#define GEOMASTER_H

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
 * Proxy class for interface org.osm.scout.server1.geomaster
 */
class GeoMaster: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return DBUS_INTERFACE_GEOMASTER; }

public:
    GeoMaster(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~GeoMaster();

    Q_PROPERTY(bool warnLargeRamLangNotSpecified READ warnLargeRamLangNotSpecified
               NOTIFY warnLargeRamLangNotSpecifiedChanged)
    inline bool warnLargeRamLangNotSpecified() const
    { return qvariant_cast< bool >(property("warnLargeRamLangNotSpecified")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void warnLargeRamLangNotSpecifiedChanged(bool warning);
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::GeoMaster geomaster;
      }
    }
  }
}
#endif
