#include "appsettings.h"

#include <QThread>

AppSettings::AppSettings():
    QSettings()
{
}

#define CHECK(s, d) if (!contains(s)) setValue(s, d);
void AppSettings::initDefaults()
{
    // to keep QtWebApp library the same as upstream, these defaults are defined here

    beginGroup("http-listener");
    CHECK("host", "127.0.0.1");
    CHECK("port", 8553);
    CHECK("maxThreads", QThread::idealThreadCount() + 2);
    endGroup();
}
