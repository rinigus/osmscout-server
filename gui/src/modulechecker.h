/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2016-2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MODULECHECKER_H
#define MODULECHECKER_H

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
 * Proxy class for interface org.osm.scout.server1.modules
 */
class ModuleChecker: public QDBusAbstractInterface
{
  Q_OBJECT
public:
  static inline const char *staticInterfaceName()
  { return DBUS_INTERFACE_MODULES; }

public:
  ModuleChecker(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

  ~ModuleChecker();

  Q_PROPERTY(bool fonts READ fonts)
  inline bool fonts() const
  { return qvariant_cast< bool >(property("fonts")); }

  Q_PROPERTY(bool valhallaRoute READ valhallaRoute)
  inline bool valhallaRoute() const
  { return qvariant_cast< bool >(property("valhallaRoute")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
};

namespace org {
  namespace osm {
    namespace scout {
      namespace server1 {
        typedef ::ModuleChecker modules;
      }
    }
  }
}
#endif
