/*
 * This file is part of OSM Scout Server.
 *
 * SPDX-FileCopyrightText: 2021 Rinigus https://github.com/rinigus
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef IDLETRACKER_H
#define IDLETRACKER_H

#include <QObject>
#include <QTimer>

#include <time.h>

class IdleTracker : public QObject
{
  Q_OBJECT

public:
  explicit IdleTracker(QObject *parent = nullptr);

  void setDBusOnly();

  void registerCall();

  void onSettingsChanged();
  void onServiceDisappeared(QString service);

signals:
  void idleTimeout();

protected:
  void checkIdle();

private:
  QTimer m_timer;
  struct timespec m_last_call;
  double m_idle_timeout;
};

#endif // IDLETRACKER_H
