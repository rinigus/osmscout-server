#include "modulechecker.h"

#include <QFileInfo>

ModuleChecker::ModuleChecker(QObject *parent) : QObject(parent)
{  
#ifdef USE_MAPNIK
  { // fonts module
    QFileInfo f(MAPNIK_FONTS_DIR);
    m_fonts = f.exists();
  }
#endif

#ifdef USE_VALHALLA
  {
    QFileInfo f1(VALHALLA_EXECUTABLE);
    QFileInfo f2(VALHALLA_CONFIG_TEMPLATE);
    m_valhalla_route = (f1.exists() && f2.exists());
  }
#endif
}
