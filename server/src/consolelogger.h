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

#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include <QObject>

class ConsoleLogger : public QObject
{
  Q_OBJECT
public:
  explicit ConsoleLogger(QObject *parent = 0);

public slots:
  void onErrorMessage(QString info);

protected:
  void log(QString txt);
};

#endif // CONSOLELOGGER_H
