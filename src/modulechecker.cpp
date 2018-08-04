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

#include "modulechecker.h"
#include "appsettings.h"
#include "config.h"

#include <QFileInfo>

ModuleChecker::ModuleChecker(QObject *parent) : QObject(parent)
{
  checkModules();
}

void ModuleChecker::onSettingsChanged()
{
  checkModules();
}

void ModuleChecker::checkModules()
{
  AppSettings settings;
  bool available_mapnik = settings.valueBool(MAPMANAGER_SETTINGS "mapnik");

#ifdef USE_MAPNIK
  { // fonts module
    QFileInfo f(MAPNIK_FONTS_DIR);
    m_fonts = !available_mapnik || f.exists();
  }
#endif

#ifdef USE_VALHALLA
  {
    // Valhalla is build in
    m_valhalla_route = true;
  }
#endif

  emit modulesChanged();
}
