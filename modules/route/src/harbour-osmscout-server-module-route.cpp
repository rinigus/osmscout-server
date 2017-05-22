/*
  Copyright (C) 2017 rinigus <rinigus.git@gmail.com>
  License: MIT
*/

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>

extern int valhalla_route_service_main(int argc, char** argv);
int main(int argc, char *argv[])
{
  if (argc > 1)
    return valhalla_route_service_main(argc, argv);

  return SailfishApp::main(argc, argv);
}
