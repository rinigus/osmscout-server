#ifndef MAPMANAGER_DELETERTHREAD_H
#define MAPMANAGER_DELETERTHREAD_H

#include <QThread>
#include <QStringList>
#include <QDir>

namespace MapManager {

  class DeleterThread : public QThread
  {
    Q_OBJECT
  public:
    DeleterThread(QObject *parent,
                  QDir &root_dir,
                  const QStringList &todelete):
      QThread(parent),
      m_root_dir(root_dir),
      m_todelete(todelete)
    {}

  protected:
    virtual void run() override;

  protected:
    QDir m_root_dir;
    QStringList m_todelete;
  };

}
#endif // DELETERTHREAD_H
