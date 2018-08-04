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

#ifndef INFOHUB_H
#define INFOHUB_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QFile>

class InfoHub : public QObject
{
  Q_OBJECT

  //////////////////////////////////////////////////////////
  /// when true, an error has been detected that has to be fixed
  /// before the server can proceed with its work. For example,
  /// database is missing
  Q_PROPERTY(bool error READ error NOTIFY errorChanged)

  /////////////////////////////////////////////////////////
  /// number of jobs in running queue (running and waiting)
  Q_PROPERTY(int queue READ queue NOTIFY queueChanged)

public:
  explicit InfoHub(QObject *parent = 0);

  static void setError(bool e);
  bool error();

  static void logInfo(const QString &txt, bool force=false);
  static void logWarning(const QString &txt);
  static void logError(const QString &txt);

  static void addJobToQueue();
  static void removeJobFromQueue();

  static void activity();

  int queue();

signals:
  void errorChanged(bool error);
  void queueChanged(int queue);

  void log(QString log);
  void info(QString info);
  void warning(QString warning);
  void error(QString error);

  void activitySig();

public slots:
  void onSettingsChanged();

protected:
  void impSetError(bool e);
  void impLogInfo(const QString &txt, bool force=false);
  void impLogWarning(const QString &txt);
  void impLogError(const QString &txt);
  void sessionLog(const QString &txt);

  void implChangeQueue(int delta);
  void implActivity();

protected:
  QMutex m_mutex;
  bool m_error = false;
  bool m_log_info = true;
  bool m_log_session = false;
  int m_queue = 0;

  QFile m_log_file;
};

#endif // INFOHUB_H
