/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

