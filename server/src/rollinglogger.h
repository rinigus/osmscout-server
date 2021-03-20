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

#ifndef ROLLINGLOGGER_H
#define ROLLINGLOGGER_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QQueue>

class RollingLogger : public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_LOGGER)

  //////////////////////////////////////////////////////////
  /// Gives the last events in a form of a QString
  Q_PROPERTY(QString log READ log NOTIFY logChanged)

public:
  explicit RollingLogger(QObject *parent = 0);

  const QString& log() const { return m_log_txt; }

signals:
  void logChanged(QString);

public slots:
  void onSettingsChanged();

protected:
  void logEntry(const QString &txt);

protected:
  int m_size_max = 20;

  QQueue<QString> m_log;
  QString m_log_txt;
};

#endif // ROLLINGLOGGER_H
