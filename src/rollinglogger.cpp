#include "rollinglogger.h"
#include "infohub.h"
#include "appsettings.h"
#include "config.h"

extern InfoHub infoHub;

RollingLogger::RollingLogger(QObject *parent) : QObject(parent)
{
    connect( &infoHub, &InfoHub::log,
             this, &RollingLogger::logEntry );
}

void RollingLogger::onSettingsChanged()
{
    AppSettings settings;

    m_size_max = settings.valueInt(GENERAL_SETTINGS "rollingLoggerSize");

    while (m_log.size() > m_size_max && m_log.size() > 1)
        m_log.dequeue();
}

void RollingLogger::logEntry(const QString &txt)
{
    while (m_log.size() >= m_size_max && m_log.size() > 1)
        m_log.dequeue();

    m_log.enqueue(txt);

    m_log_txt.clear();
    foreach (const QString a, m_log)
        m_log_txt = a + "\n" + m_log_txt;

    emit logChanged(m_log_txt);
}

