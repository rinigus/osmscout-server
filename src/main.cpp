/**
  @file
  @author Stefan Frings
*/

#include <QGuiApplication>
#include <QDir>

// QtWebApp header
#include "httplistener.h"
#include "requestmapper.h"

#include "dbmaster.h"

DBMaster *osmScoutMaster = NULL;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc,argv);

    // setup OSM Scout
    osmScoutMaster = new DBMaster("map",
                                  "stylesheets/standard.oss");

    app.setApplicationName("osmscout-server");
    app.setOrganizationName("osmscout-server");

    // Configure and start the TCP listener
    QSettings* settings=new QSettings;

    settings->beginGroup("http-listener");

    // check and init defaults if settings are absent
    if (!settings->contains("port"))
        settings->setValue("port", 8080);

    if (!settings->contains("maxThreads"))
        settings->setValue("maxThreads", QThread::idealThreadCount() + 2);

    new HttpListener(settings,new RequestMapper(&app),&app);
    settings->endGroup();

    qWarning("Application has started");

    app.exec();

    qWarning("Application has stopped");

    return 0;
}
