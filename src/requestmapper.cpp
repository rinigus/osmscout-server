/**
  @file
  @author Stefan Frings
*/

#include <QPixmap>
#include <QBuffer>
#include "requestmapper.h"
#include "dbmaster.h"

extern DBMaster *osmScoutMaster;

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent)
{
    qDebug("RequestMapper: created");
}


RequestMapper::~RequestMapper()
{
    qDebug("RequestMapper: deleted");
}

// Helper function checking if sep is in path. If it is, it parses the section after
// sep from path by splitting it using '/' as a separator. Example:
//    path = "http://localhost:8553/v1/tile/1/2/3
//    sep = "/v1/tile/"
//    return true and fill command with "1", "2", "3"
static bool contains(const QString &path, const QString &sep, QStringList &command)
{
    if (path.contains(sep))
    {
        QString section = path.section(sep, 1);
        command = section.split('/');
        return true;
    }
    return false;
}

int long2tilex(double lon, int z)
{
    return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

int lat2tiley(double lat, int z)
{
    return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}

double tilex2long(int x, int z)
{
    return x / pow(2.0, z) * 360.0 - 180;
}

double tiley2lat(int y, int z)
{
    double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void returnError(HttpResponse &response)
{
    response.setStatus(500, "unknown command");
    response.write("Please read the source for command syntax", true);
}

void RequestMapper::service(HttpRequest& request, HttpResponse& response)
{
    QString path=request.getPath();
    qDebug("RequestMapper: path=%s", path.toStdString().c_str());

    QStringList command;
    if (contains(path, "/v1/tile/", command)) // Tile
    {
        int last_comm = command.size()-1;
        if ( command.size() != 6 ||
                !command[last_comm].endsWith(".png") )
        {
            returnError(response);
            return;
        }

        // remove .png from the last command
        command[last_comm].resize( command[last_comm].size() - 4 /*size of .png */);

        // if you modify syntax here, don't forget to change the check on command size above
        bool daylight = ( command[0].toInt() > 0 );
        int shift = command[1].toInt();
        int scale = command[2].toInt();
        int z = command[3].toInt();
        int x = command[4].toInt();
        int y = command[5].toInt();

        int ntiles = 1 << shift;

        QByteArray bytes;
        if ( !osmScoutMaster->renderMap(daylight, 96*scale/ntiles, z + shift, 256*scale, 256*scale,
                                        (tiley2lat(y, z) + tiley2lat(y+1, z))/2.0,
                                        (tilex2long(x, z) + tilex2long(x+1, z))/2.0, bytes ) )
        {
            returnError(response);
            return;
        }

        response.setHeader("Content-Type", "image/png");
        response.write(bytes, true);
    }

    else // command unidentified. return help string
    {
        returnError(response);
        return;
    }
}
