/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "microhttpservicebase.h"
#include <QThreadPool>

/**
  The request mapper dispatches incoming HTTP requests to controller classes
  depending on the requested path.
*/

class RequestMapper : public MicroHTTP::ServiceBase
{
public:

    RequestMapper();
    virtual ~RequestMapper();

    /**
      Dispatch incoming HTTP requests to different controllers depending on the URL.
    */
    virtual unsigned int service(const char *url, MHD_Connection *, MHD_Response *,
                                 MicroHTTP::Connection::keytype connection_id);
    virtual void loguri(const char *uri);

protected:
    QThreadPool m_pool;
};

#endif // REQUESTMAPPER_H
