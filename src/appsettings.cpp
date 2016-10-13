#include "appsettings.h"
#include "config.h"

#include <QThread>
#include <QStandardPaths>
#include <QDebug>

#ifdef IS_SAILFISH_OS
#define DATA_PREFIX "/usr/share/harbour-osmscout-server/"
#endif

#ifdef IS_CONSOLE_QT
#define DATA_PREFIX ""
#endif

AppSettings::AppSettings():
    QSettings()
{
}

#define CHECK(s, d) if (!contains(s)) QSettings::setValue(s, d);
void AppSettings::initDefaults()
{
    // defaults for server
    beginGroup("http-listener");
    CHECK("host", "127.0.0.1");
    CHECK("port", 8553);
    CHECK("maxThreads", QThread::idealThreadCount() + 2);
    endGroup();

    // defaults for libosmscout
#ifdef IS_SAILFISH_OS
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    CHECK(OSM_SETTINGS "map", documents + "/Maps");
#else
    CHECK(OSM_SETTINGS "map", "map");
#endif

    CHECK(OSM_SETTINGS "style", DATA_PREFIX "stylesheets/standard.oss");
    CHECK(OSM_SETTINGS "icons", DATA_PREFIX "data/icons/28x28/standard/");
    CHECK(OSM_SETTINGS "fontSize", 5.0);
    CHECK(OSM_SETTINGS "renderSea", 1);
    CHECK(OSM_SETTINGS "drawBackground", 1);

    CHECK(OSM_SETTINGS "rollingLoggerSize", 10);
    CHECK(OSM_SETTINGS "logInfo", 1);
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);

    if (key.contains(OSM_SETTINGS))
        emit osmScoutSettingsChanged();
}


int AppSettings::valueInt(const QString &key)
{
    return value(key, 0).toInt();
}

int AppSettings::valueFloat(const QString &key)
{
    return value(key, 0).toFloat();
}

QString AppSettings::valueString(const QString &key)
{
    return value(key, QString()).toString();
}
