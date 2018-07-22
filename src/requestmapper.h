#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "microhttpservicebase.h"

#include <QThreadPool>
#include <QTimer>
#include <QObject>
#include <QString>

#include <time.h>

/**
  The request mapper dispatches incoming HTTP requests to controller classes
  depending on the requested path.
*/

class RequestMapper : public QObject, public MicroHTTP::ServiceBase
{
  Q_OBJECT

public:

  explicit RequestMapper(QObject *parent = 0);
  virtual ~RequestMapper();

  /**
      Dispatch incoming HTTP requests to different controllers depending on the URL.
    */
  virtual unsigned int service(const char *url, MHD_Connection *, MHD_Response *,
                               MicroHTTP::Connection::keytype connection_id);
  virtual void loguri(const char *uri);

signals:
  void idleTimeout();

public slots:
  void onSettingsChanged();
  void checkIdle();
  void updateLastCall();

protected:
  QThreadPool m_pool;
  QTimer m_timer;
  struct timespec m_last_call;
  double m_idle_timeout;

  // availibility of different backends
  bool m_available_geocodernlp{false};
  bool m_available_mapboxgl{false};
  bool m_available_mapnik{false};
  bool m_available_osmscout{false};
  bool m_available_valhalla{false};

  QString m_info_enable_backends;
};

#endif // REQUESTMAPPER_H
