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
  bool available_valhalla = settings.valueBool(MAPMANAGER_SETTINGS "valhalla");

#ifdef USE_MAPNIK
  { // fonts module
    QFileInfo f(MAPNIK_FONTS_DIR);
    m_fonts = !available_mapnik || f.exists();
  }
#endif

#ifdef USE_VALHALLA
  {
    QFileInfo f1(VALHALLA_EXECUTABLE);
    QFileInfo f2(VALHALLA_CONFIG_TEMPLATE);
    m_valhalla_route = !available_valhalla || (f1.exists() && f2.exists());
  }
#endif

  emit modulesChanged();
}
