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

  m_valhalla_route = true;
}
