#include "appsettings.h"
#include "config.h"

#include <QThread>
#include <QStandardPaths>
#include <QTimer>

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
#define CHECKF(s, d) if (!contains(s)) QSettings::setValue(s, (double)d);
void AppSettings::initDefaults()
{
    // defaults for server
    beginGroup("http-listener");
    CHECK("host", "127.0.0.1");
    CHECK("port", 8553);
    //CHECK("maxThreads", QThread::idealThreadCount() + 2);
    endGroup();

    // defaults for libosmscout
#ifdef IS_SAILFISH_OS
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    CHECK(OSM_SETTINGS "map", documents + "/Maps");
#else
    CHECK(OSM_SETTINGS "map", "map");
#endif

    CHECK(OSM_SETTINGS "style", DATA_PREFIX "stylesheets/standard.oss");
    CHECK(OSM_SETTINGS "icons", DATA_PREFIX "data/icons/28x28/standard");
    CHECK(OSM_SETTINGS "fontSize", 5.0);
    CHECK(OSM_SETTINGS "renderSea", 1);
    CHECK(OSM_SETTINGS "drawBackground", 1);
    CHECK(OSM_SETTINGS "dataLookupArea", 1.25);
    CHECK(OSM_SETTINGS "tileBordersZoomCutoff", 16);

    CHECK(OSM_SETTINGS "rollingLoggerSize", 10);
    CHECK(OSM_SETTINGS "logInfo", 1);
    CHECK(OSM_SETTINGS "logSession", 0);

    CHECK(OSM_SETTINGS "routingCostLimitDistance", 50.0);
    CHECK(OSM_SETTINGS "routingCostLimitFactor", 5.0);

    CHECK(ROUTING_SPEED_SETTINGS "highway_living_street", 10);
    CHECK(ROUTING_SPEED_SETTINGS "highway_motorway", 110);
    CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_junction", 60);
    CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_link", 60);
    CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_primary", 70);
    CHECK(ROUTING_SPEED_SETTINGS "highway_motorway_trunk", 100);
    CHECK(ROUTING_SPEED_SETTINGS "highway_primary", 70);
    CHECK(ROUTING_SPEED_SETTINGS "highway_primary_link", 60);
    CHECK(ROUTING_SPEED_SETTINGS "highway_residential", 40);
    CHECK(ROUTING_SPEED_SETTINGS "highway_road", 50);
    CHECK(ROUTING_SPEED_SETTINGS "highway_roundabout", 40);
    CHECK(ROUTING_SPEED_SETTINGS "highway_secondary", 60);
    CHECK(ROUTING_SPEED_SETTINGS "highway_secondary_link", 50);
    CHECK(ROUTING_SPEED_SETTINGS "highway_service", 30);
    CHECK(ROUTING_SPEED_SETTINGS "highway_tertiary", 55);
    CHECK(ROUTING_SPEED_SETTINGS "highway_tertiary_link", 55);
    CHECK(ROUTING_SPEED_SETTINGS "highway_trunk", 100);
    CHECK(ROUTING_SPEED_SETTINGS "highway_trunk_link", 60);
    CHECK(ROUTING_SPEED_SETTINGS "highway_unclassified", 50);
    CHECK(ROUTING_SPEED_SETTINGS "Bicycle", 20);
    CHECK(ROUTING_SPEED_SETTINGS "Foot", 5);
    CHECK(ROUTING_SPEED_SETTINGS "Car", 160);

    // Fix icons dir setting if coming from earlier versions.
    QString icons = valueString(OSM_SETTINGS "icons");
    if (icons.size() > 1 && icons.at(icons.size()-1) == '/')
    {
        qDebug() << "Looks like icons path has trailing /: " << icons;
        icons.chop(1);
        qDebug() << "New icons path: " << icons;
        setValue(OSM_SETTINGS "icons", icons);
    }
}

void AppSettings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);

    if (key.contains(OSM_SETTINGS) || key.contains(ROUTING_SPEED_SETTINGS))
    {
        // this delayed signal execution prevents fireing signals together
        // if there are many changes in settings
        if (!m_signal_osm_scout_changed_waiting)
        {
            m_signal_osm_scout_changed_waiting = true;
            QTimer::singleShot(200, this, SLOT(fireOsmScoutSettingsChanged()));
        }
    }
}

void AppSettings::fireOsmScoutSettingsChanged()
{
    emit osmScoutSettingsChanged();
    m_signal_osm_scout_changed_waiting = false;
    //qDebug() << "Signal fired";
}


int AppSettings::valueInt(const QString &key)
{
    return value(key, 0).toInt();
}

double AppSettings::valueFloat(const QString &key)
{
    return value(key, 0).toFloat();
}

QString AppSettings::valueString(const QString &key)
{
    return value(key, QString()).toString();
}

