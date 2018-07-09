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
