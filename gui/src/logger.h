/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGGER_H
#define LOGGER_H

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
 * Proxy class for interface org.osm.scout.server1.logger
 */
class Logger: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return DBUS_INTERFACE_LOGGER; }

public:
    Logger(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~Logger();

    Q_PROPERTY(QString log READ log NOTIFY logChanged)
    inline QString log() const
    { return qvariant_cast< QString >(property("log")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void logChanged(); //const QString &in0);
    void reloadData();
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::Logger logger;
      }
    }
  }
}
#endif
