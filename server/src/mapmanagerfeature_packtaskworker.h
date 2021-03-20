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

#ifndef MAPMANAGERFEATURE_PACKTASKWORKER_H
#define MAPMANAGERFEATURE_PACKTASKWORKER_H

#include <QObject>
#include <QThread>
#include <QString>

class PackTaskWorker: public QThread
{
  Q_OBJECT

public:
  PackTaskWorker(const QString &fname, const QString &dtime,
                 const QString &maindir,
                 const QString &tiledir,
                 const QString &tiledtime):
    QThread(),
    filename(fname), datetime(dtime),
    maindirname(maindir),
    tiledirname(tiledir),
    tiledatetime(tiledtime) {}

  virtual ~PackTaskWorker() {};

signals:
  void errorDuringTask(QString errtxt);

protected:

  virtual void run() override;

protected:
  QString filename;
  QString datetime;
  QString maindirname;
  QString tiledirname;
  QString tiledatetime;
};

#endif // MAPMANAGERFEATURE_PACKTASKWORKER_H
