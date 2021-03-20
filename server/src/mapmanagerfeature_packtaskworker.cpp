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

#include "mapmanagerfeature_packtaskworker.h"
#include "infohub.h"

#include <QDir>
#include <QCoreApplication>
#include <QProcess>

#include <QDebug>

void PackTaskWorker::run()
{
  if (datetime != tiledatetime)
    {
      QDir tiledir(tiledirname);
      if (!tiledir.removeRecursively())
        {
          emit errorDuringTask( QCoreApplication::translate("MapManagerFeature", "Error deleting Valhalla's tile directory %1").arg(tiledirname) );
          return;
        }
    }

  // unpack
  QProcess unpacker;
  unpacker.setWorkingDirectory(maindirname);
  QString prog = "tar";
  QStringList args;
  args << "xf"
       << filename;
  unpacker.start(prog, args);

  if (!unpacker.waitForStarted())
    {
      emit errorDuringTask( QCoreApplication::translate("MapManagerFeature",
                                                        "Error while starting unpacking program") );
      return;
    }

  while (!unpacker.waitForFinished())
    {
      if ( unpacker.error() != QProcess::Timedout )
        {
          if ( unpacker.state() != QProcess::Running )
            {
              emit errorDuringTask( QCoreApplication::translate("MapManagerFeature",
                                                                "Error during unpacking Valhalla's package") );
              return;
            }
        }
    }

  InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature",
                                               "Unpacked Valhalla's package: %1").arg(filename));

  // delete package
  QDir dir(maindirname);
  if (!dir.remove(filename))
    InfoHub::logInfo(QCoreApplication::translate("MapManagerFeature", "Failed to remove unnecessary file: %1").arg(filename));
}
