#include "infohub.h"
#include "config.h"
#include "appsettings.h"

#include <QMutexLocker>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>

#include <QDebug>

#include <iostream>

InfoHub infoHub;

InfoHub::InfoHub(QObject *parent) : QObject(parent)
{
    connect( this, &InfoHub::log, this, &InfoHub::sessionLog );
}

void InfoHub::onSettingsChanged()
{
    AppSettings settings;

    m_log_info = (settings.valueInt(OSM_SETTINGS "logInfo") > 0);
    m_log_session = (settings.valueInt(OSM_SETTINGS "logSession") > 0);
}

void InfoHub::setError(bool e)
{
    infoHub.impSetError(e);
}

void InfoHub::impSetError(bool e)
{
    bool ce;
    {
        QMutexLocker lk(&m_mutex);
        ce = m_error;
        m_error =  e;
    }
    if (ce != e)
        emit errorChanged(e);
}

bool InfoHub::error()
{
    QMutexLocker lk(&m_mutex);
    return m_error;
}

void InfoHub::addJobToQueue()
{
    infoHub.implChangeQueue(+1);
}

void InfoHub::removeJobFromQueue()
{
    infoHub.implChangeQueue(-1);
}

void InfoHub::implChangeQueue(int delta)
{
    {
        QMutexLocker lk(&m_mutex);
        m_queue += delta;
    }
    emit queueChanged(m_queue);
}

int InfoHub::queue()
{
    QMutexLocker lk(&m_mutex);
    return m_queue;
}

static QString tstamp(const QString &txt)
{
    return QDateTime::currentDateTime().toString("HH:mm:ss") + " " + txt;
}

void InfoHub::logError(const QString &txt)
{
    infoHub.impLogError(tstamp(txt));
}

void InfoHub::logWarning(const QString &txt)
{
    infoHub.impLogWarning(tstamp(txt));
}

void InfoHub::logInfo(const QString &txt, bool force)
{
    infoHub.impLogInfo(tstamp(txt), force);
}

void InfoHub::impLogError(const QString &txt)
{
    emit error(txt);
    emit log(tr("ERROR: ") + txt);
}

void InfoHub::impLogWarning(const QString &txt)
{
    emit warning(txt);
    emit log(tr("WARNING: ") + txt);
}

void InfoHub::impLogInfo(const QString &txt, bool force)
{
    if (!force && !m_log_info && !m_log_session)
        return;

    emit info(txt);
    emit log(tr("INFO: ") + txt);
}

void InfoHub::sessionLog(const QString &txt)
{
    if (!m_log_session)
        return;

    if ( !m_log_file.isOpen() )
    {
        QDir dir;
        QString dirpath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        if (!dir.mkpath(dirpath))
        {
            std::cerr << "Cannot make directory " << dirpath.toStdString() << std::endl;
            return;
        }

        m_log_file.setFileName(dirpath + "/session.log");
        if (!m_log_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            std::cerr << "Error opening file " << m_log_file.fileName().toStdString() << std::endl;
            return;
        }
    }

    QTextStream out(&m_log_file);
    out << txt << "\n";
}
