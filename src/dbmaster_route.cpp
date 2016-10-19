#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <osmscout/RoutingService.h>
#include <osmscout/RoutePostprocessor.h>

#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

/////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions

static void GetCarSpeedTable(std::map<std::string,double>& map)
{
  map["highway_motorway"]=110.0;
  map["highway_motorway_trunk"]=100.0;
  map["highway_motorway_primary"]=70.0;
  map["highway_motorway_link"]=60.0;
  map["highway_motorway_junction"]=60.0;
  map["highway_trunk"]=100.0;
  map["highway_trunk_link"]=60.0;
  map["highway_primary"]=70.0;
  map["highway_primary_link"]=60.0;
  map["highway_secondary"]=60.0;
  map["highway_secondary_link"]=50.0;
  map["highway_tertiary_link"]=55.0;
  map["highway_tertiary"]=55.0;
  map["highway_unclassified"]=50.0;
  map["highway_road"]=50.0;
  map["highway_residential"]=40.0;
  map["highway_roundabout"]=40.0;
  map["highway_living_street"]=10.0;
  map["highway_service"]=30.0;
}


/////////////////////////////////////////////////////////////////////////////////////////
/// Main routing function
bool DBMaster::route(osmscout::Vehicle &vehicle, std::vector<osmscout::GeoCoord> &via, double radius, QByteArray &result)
{
    ///////////////////////////////////////////////////////////
    /// Check if everything is OK and lock the mutex
    ///////////////////////////////////////////////////////////
    if (m_error_flag) return false;

    QMutexLocker lk(&m_mutex);

    if (!m_database->IsOpen())
    {
        InfoHub::logWarning("Database is not open, cannot route");
        return false;
    }

    ///////////////////////////////////////////////////////////
    /// Routing
    ///////////////////////////////////////////////////////////

    std::string                         routerFilenamebase=osmscout::RoutingService::DEFAULT_FILENAME_BASE;
    osmscout::FastestPathRoutingProfile routingProfile(m_database->GetTypeConfig());
    osmscout::RouterParameter           routerParameter;

    routerParameter.SetDebugPerformance(true);

    osmscout::RoutingServiceRef router=std::make_shared<osmscout::RoutingService>(m_database,
                                                                                  routerParameter,
                                                                                  routerFilenamebase);

    if (!router->Open())
    {
      InfoHub::logWarning("Cannot open routing database");
      return false;
    }

    osmscout::TypeConfigRef             typeConfig=m_database->GetTypeConfig();
    osmscout::RouteData                 data;
    osmscout::RouteDescription          description;
    std::map<std::string,double>        carSpeedTable;

    switch (vehicle)
    {
    case osmscout::vehicleFoot:
        routingProfile.ParametrizeForFoot(*typeConfig,
                                          5.0);
        break;
    case osmscout::vehicleBicycle:
        routingProfile.ParametrizeForBicycle(*typeConfig,
                                             20.0);
        break;
    case osmscout::vehicleCar:
        GetCarSpeedTable(carSpeedTable);
        routingProfile.ParametrizeForCar(*typeConfig,
                                         carSpeedTable,
                                         160.0);
        break;
    }

    if (!router->CalculateRoute(routingProfile,
                                vehicle,
                                radius,
                                via,
                                data))
    {
      InfoHub::logWarning("There was an error while calculating the route!");
      router->Close();
      return false;
    }

    if (data.IsEmpty())
    {
      InfoHub::logWarning("No Route found!");
      router->Close();
      return false;
    }

    /// Route points
    std::list<osmscout::Point> route_points;
    if (!router->TransformRouteDataToPoints(data,
                                            route_points))
    {
      InfoHub::logWarning("Error during route conversion to points");
      router->Close();
      return false;
    }

    router->TransformRouteDataToRouteDescription(data,
                                                 description);

    std::list<osmscout::RoutePostprocessor::PostprocessorRef> postprocessors;

    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::DistanceAndTimePostprocessor>());
    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::StartPostprocessor>("Start"));
    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::TargetPostprocessor>("Target"));
    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::WayNamePostprocessor>());
    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::CrossingWaysPostprocessor>());
    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::DirectionPostprocessor>());
    postprocessors.push_back(std::make_shared<osmscout::RoutePostprocessor::MotorwayJunctionPostprocessor>());

    osmscout::RoutePostprocessor::InstructionPostprocessorRef instructionProcessor=std::make_shared<osmscout::RoutePostprocessor::InstructionPostprocessor>();

    instructionProcessor->AddMotorwayType(typeConfig->GetTypeInfo("highway_motorway"));
    instructionProcessor->AddMotorwayLinkType(typeConfig->GetTypeInfo("highway_motorway_link"));
    instructionProcessor->AddMotorwayType(typeConfig->GetTypeInfo("highway_motorway_trunk"));
    instructionProcessor->AddMotorwayType(typeConfig->GetTypeInfo("highway_motorway_primary"));
    instructionProcessor->AddMotorwayType(typeConfig->GetTypeInfo("highway_trunk"));
    instructionProcessor->AddMotorwayLinkType(typeConfig->GetTypeInfo("highway_trunk_link"));
    postprocessors.push_back(instructionProcessor);

    osmscout::RoutePostprocessor postprocessor;
    size_t                       roundaboutCrossingCounter=0;

    if (!postprocessor.PostprocessRouteDescription(description,
                                                   routingProfile,
                                                   *m_database,
                                                   postprocessors))
    {
        InfoHub::logWarning("Error during post-processing route description");
        router->Close();
        return false;
    }

    ////////////////////////////////////////////////////////////////////////
    /// Store results

    QJsonObject rootObj; /// result JSON
    {   /// points used to calculate the route
        QJsonArray main_points;
        for (const osmscout::GeoCoord &p: via)
        {
            QJsonObject po;
            po.insert("lat", p.GetLat());
            po.insert("lng", p.GetLon());
            main_points.append(po);
        }

        rootObj.insert("points", main_points);
    }

    {   /// route in coordinates
        QJsonArray lat;
        QJsonArray lon;

        for (const osmscout::Point &p : route_points)
        {
            lat.push_back(p.GetLat());
            lon.push_back(p.GetLon());
        }

        rootObj.insert("lat", lat);
        rootObj.insert("lng", lon);
    }



    QJsonDocument document(rootObj);
    result = document.toJson();

    router->Close();
    return true;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// AS GPX
    ///////////////////////////////////////////////////////////////////////////////////////////////////
//    QTextStream output(&result, QIODevice::WriteOnly);
//    output.setRealNumberPrecision(8);

//    output << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" << "\n";
//    output << "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"bin2gpx\" version=\"1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">" << "\n";

//    output << "\t<wpt lat=\""<< via[0].GetLat() << "\" lon=\""<< via[0].GetLon() << "\">" << "\n";
//    output << "\t\t<name>Start</name>" << "\n";
//    output << "\t\t<fix>2d</fix>" << "\n";
//    output << "\t</wpt>" << "\n";

//    output << "\t<wpt lat=\""<< via[via.size()-1].GetLat() << "\" lon=\""<< via[via.size()-1].GetLon() << "\">" << "\n";
//    output << "\t\t<name>Target</name>" << "\n";
//    output << "\t\t<fix>2d</fix>" << "\n";
//    output << "\t</wpt>" << "\n";

//    output << "\t<trk>" << "\n";
//    output << "\t\t<name>Route</name>" << "\n";
//    output << "\t\t<trkseg>" << "\n";
//    for (const auto &point : points)
//    {
//      output << "\t\t\t<trkpt lat=\""<< point.GetLat() << "\" lon=\""<< point.GetLon() <<"\">" << "\n";
//      output << "\t\t\t\t<fix>2d</fix>" << "\n";
//      output << "\t\t\t</trkpt>" << "\n";
//    }
//    output << "\t\t</trkseg>" << "\n";
//    output << "\t</trk>" << "\n";
//    output << "</gpx>" << "\n";
//    router->Close();
//    return true;
}
