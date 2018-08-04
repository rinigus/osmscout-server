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

#ifndef SYSTEMDSERVICE_H
#define SYSTEMDSERVICE_H

#include <QObject>

#include <QString>

/// \brief Interface with SystemD socket activation service
///
/// This interface allows to enable/disable and start/stop of the
/// socket controlling the server daemon
class SystemDService : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged) ///< when true, socket activation of the service is enabled on boot

public:

  explicit SystemDService(QObject *parent = nullptr);

  Q_INVOKABLE bool enabled() const { return m_enabled; }
  Q_INVOKABLE void setEnabled(bool e); ///< Enable socket activation

  Q_INVOKABLE void update(); ///< Update state of the service

  void start();           ///< Start socket activation
  void stop();            ///< Stop socket activation and activated service

signals:
  void enabledChanged();

protected:
  void action_helper(const QString &action, const QString &ext);

protected:
  bool m_enabled{false};
  QString m_extra{"--user"};  ///< additional command line argument to systemctl command

  const QString const_service_name{"osmscout-server"};
};

#endif // SYSTEMDSERVICE_H
