/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INFOHUB_H
#define INFOHUB_H

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
 * Proxy class for interface org.osm.scout.server1.infohub
 */
class InfoHub: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return DBUS_INTERFACE_INFOHUB; }

public:
    InfoHub(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~InfoHub();

    Q_PROPERTY(bool error READ error NOTIFY errorChanged)
    inline bool error() const
    { return qvariant_cast< bool >(property("error")); }

    Q_PROPERTY(int queue READ queue NOTIFY queueChanged)
    inline int queue() const
    { return qvariant_cast< int >(property("queue")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void errorChanged(bool error);
    void queueChanged(int queue);
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::InfoHub infohub;
      }
    }
  }
}
#endif
