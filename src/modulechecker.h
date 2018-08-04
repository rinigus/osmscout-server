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

#ifndef MODULECHECKER_H
#define MODULECHECKER_H

#include <QObject>

/// \brief Checks availability of the installed modules
///
/// If everything is fine with the module (either installed or
/// not required) then corresponding variable is set to true. If
/// the installation of the module is needed, it is set to false.
///
class ModuleChecker : public QObject
{
  Q_OBJECT

  /// \brief Indicates whether fonts module has been installed
  Q_PROPERTY(bool fonts READ fonts)

  Q_PROPERTY(bool valhallaRoute READ valhallaRoute)

public:
  explicit ModuleChecker(QObject *parent = 0);

  bool fonts() const { return m_fonts; }
  bool valhallaRoute() const { return m_valhalla_route; }

signals:
  void modulesChanged();

public slots:
  void onSettingsChanged();

protected:
  void checkModules();

protected:
  bool m_fonts{true};
  bool m_valhalla_route{true};
};

#endif // MODULECHECKER_H
