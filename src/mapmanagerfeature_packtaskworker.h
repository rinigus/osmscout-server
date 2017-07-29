#ifndef MAPMANAGERFEATURE_PACKTASKWORKER_H
#define MAPMANAGERFEATURE_PACKTASKWORKER_H

#include <QObject>
#include <QThread>
#include <QString>

class PackTaskWorker: public QThread
{
  Q_OBJECT

public:
  PackTaskWorker(const QString &fname, const QString &dtime,
                 const QString &maindir,
                 const QString &tiledir,
                 const QString &tiledtime):
    QThread(),
    filename(fname), datetime(dtime),
    maindirname(maindir),
    tiledirname(tiledir),
    tiledatetime(tiledtime) {}

  virtual ~PackTaskWorker() {};

signals:
  void errorDuringTask(QString errtxt);

protected:

  virtual void run() override;

protected:
  QString filename;
  QString datetime;
  QString maindirname;
  QString tiledirname;
  QString tiledatetime;
};

#endif // MAPMANAGERFEATURE_PACKTASKWORKER_H
