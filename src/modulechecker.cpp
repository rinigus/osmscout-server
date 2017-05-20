#include "modulechecker.h"

#include <QFileInfo>

ModuleChecker::ModuleChecker(QObject *parent) : QObject(parent)
{  
  { // fonts module
    QFileInfo f(MAPNIK_FONTS_DIR);
    m_fonts = f.exists();
  }

}
