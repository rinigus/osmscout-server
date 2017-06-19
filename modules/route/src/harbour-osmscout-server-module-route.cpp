/*
  Copyright (C) 2017 rinigus <rinigus.git@gmail.com>
  License: MIT
*/

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>
#include <QScopedPointer>
#include <QtQuick>
#include <QtQml>

extern int valhalla_route_service_main(int argc, char** argv);
int main(int argc, char *argv[])
{
  if (argc > 1)
    return valhalla_route_service_main(argc, argv);

  // Sailfish GUI
  QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

  QScopedPointer<QQuickView> v(SailfishApp::createView());
  QQmlContext *rootContext = v->rootContext();
  rootContext->setContextProperty("programVersion", APP_VERSION);

  v->setSource(SailfishApp::pathTo("qml/harbour-osmscout-server-module-route.qml"));
  v->show();

  return app->exec();
}
