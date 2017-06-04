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
