#ifndef SERVICEBASE_H
#define SERVICEBASE_H

#include "microhttpconnection.h"

#include <microhttpd.h>

namespace MicroHTTP {

class ServiceBase
{
public:
    virtual unsigned int service(const char *url, MHD_Connection *, MHD_Response *, MicroHTTP::Connection::keytype connection_id) = 0;
    virtual void loguri(const char *uri) {}

protected:
    ServiceBase();
    virtual ~ServiceBase() {}
};

}
#endif // SERVICEBASE_H
