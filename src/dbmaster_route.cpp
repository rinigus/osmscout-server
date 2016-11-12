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
#include <QDebug>
#include <QString>

#include <sstream>

#define H2S(x) ((x)*60.0*60.0) // hours -> seconds

/////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions

//static void GetCarSpeedTable(std::map<std::string,double>& map)
//{
//    map["highway_motorway"]=110.0;
//    map["highway_motorway_trunk"]=100.0;
//    map["highway_motorway_primary"]=70.0;
//    map["highway_motorway_link"]=60.0;
//    map["highway_motorway_junction"]=60.0;
//    map["highway_trunk"]=100.0;
//    map["highway_trunk_link"]=60.0;
//    map["highway_primary"]=70.0;
//    map["highway_primary_link"]=60.0;
//    map["highway_secondary"]=60.0;
//    map["highway_secondary_link"]=50.0;
//    map["highway_tertiary_link"]=55.0;
//    map["highway_tertiary"]=55.0;
//    map["highway_unclassified"]=50.0;
//    map["highway_road"]=50.0;
//    map["highway_residential"]=40.0;
//    map["highway_roundabout"]=40.0;
//    map["highway_living_street"]=10.0;
//    map["highway_service"]=30.0;

//    for (auto i: map)
//        std::cout << "CHECK(OSM_SETTINGS ROUTING_SPEED_SETTINGS \"" << i.first << "\", " << i.second << ");\n";
//}

static bool HasRelevantDescriptions(const osmscout::RouteDescription::Node& node)
{
    if (node.HasDescription(osmscout::RouteDescription::NODE_START_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::NODE_TARGET_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::WAY_NAME_CHANGED_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::ROUNDABOUT_ENTER_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::ROUNDABOUT_LEAVE_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::TURN_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::MOTORWAY_ENTER_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::MOTORWAY_CHANGE_DESC)) {
        return true;
    }

    if (node.HasDescription(osmscout::RouteDescription::MOTORWAY_LEAVE_DESC)) {
        return true;
    }

    return false;
}


static std::string CrossingWaysDescriptionToString(const osmscout::RouteDescription::CrossingWaysDescription& crossingWaysDescription)
{
    std::set<std::string>                          names;
    osmscout::RouteDescription::NameDescriptionRef originDescription=crossingWaysDescription.GetOriginDesccription();
    osmscout::RouteDescription::NameDescriptionRef targetDescription=crossingWaysDescription.GetTargetDesccription();

    if (originDescription) {
        std::string nameString=originDescription->GetDescription();

        if (!nameString.empty()) {
            names.insert(nameString);
        }
    }

    if (targetDescription) {
        std::string nameString=targetDescription->GetDescription();

        if (!nameString.empty()) {
            names.insert(nameString);
        }
    }

    for (std::list<osmscout::RouteDescription::NameDescriptionRef>::const_iterator name=crossingWaysDescription.GetDescriptions().begin();
         name!=crossingWaysDescription.GetDescriptions().end();
         ++name) {
        std::string nameString=(*name)->GetDescription();

        if (!nameString.empty()) {
            names.insert(nameString);
        }
    }

    if (names.size()>0) {
        std::ostringstream stream;

        for (std::set<std::string>::const_iterator name=names.begin();
             name!=names.end();
             ++name) {
            if (name!=names.begin()) {
                stream << ", ";
            }
            stream << "" << *name << "";
        }

        return stream.str();
    }
    else {
        return "";
    }
}


static void MoveToTurnCommand(osmscout::RouteDescription::DirectionDescription::Move move,
                              std::string &txt,
                              std::string &type)
{
    switch (move) {
    case osmscout::RouteDescription::DirectionDescription::sharpLeft:
        txt = "Turn sharp left"; type = "turn-sharp-left"; return;
    case osmscout::RouteDescription::DirectionDescription::left:
        txt = "Turn left"; type = "turn-left"; return;
    case osmscout::RouteDescription::DirectionDescription::slightlyLeft:
        txt = "Turn slightly left"; type = "turn-slight-left"; return;
    case osmscout::RouteDescription::DirectionDescription::straightOn:
        txt = "Straight on"; type = "straight"; return;
    case osmscout::RouteDescription::DirectionDescription::slightlyRight:
        txt = "Turn slightly right"; type = "turn-slight-right"; return;
    case osmscout::RouteDescription::DirectionDescription::right:
        txt = "Turn right"; type = "turn-right"; return;
    case osmscout::RouteDescription::DirectionDescription::sharpRight:
        txt = "Turn sharp right"; type = "turn-sharp-right"; return;
    }

    assert(false);
}


static void DumpStartDescription(QJsonObject& action,
                                 const osmscout::RouteDescription::StartDescriptionRef& startDescription,
                                 const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    QString txt = "Start at " + QString::fromStdString(startDescription->GetDescription()) + "";

    if (nameDescription &&
            nameDescription->HasName()) {
        QString post = "Drive along " + QString::fromStdString(nameDescription->GetDescription()) + "";
        action.insert("verbal_post_transition_instruction", post);
        txt += ". " + post;
    }

    action.insert("instruction", txt);
    action.insert("type", QString("start"));
}


static void DumpTargetDescription(QJsonObject& action,
                                  const osmscout::RouteDescription::TargetDescriptionRef& targetDescription)
{
    action.insert("instruction", "Target reached: " + QString::fromStdString(targetDescription->GetDescription()) + "");
    action.insert("type", QString("destination"));
}


static void DumpTurnDescription(QJsonObject& action,
                                const osmscout::RouteDescription::TurnDescriptionRef& /*turnDescription*/,
                                const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription,
                                const osmscout::RouteDescription::DirectionDescriptionRef& directionDescription,
                                const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    std::string crossingWaysString;
    std::string pre;
    std::string comm;
    std::string type;

    if (crossingWaysDescription) {
        crossingWaysString=CrossingWaysDescriptionToString(*crossingWaysDescription);
    }

    if (!crossingWaysString.empty()) {
        pre = "At crossing " + crossingWaysString;
    }

    if (directionDescription) {
        MoveToTurnCommand(directionDescription->GetCurve(), comm, type);
    }
    else {
        comm = "Turn";
    }

    if (nameDescription &&
            nameDescription->HasName()) {
        comm += " into " + nameDescription->GetDescription() + "";
    }

    action.insert("instruction", QString::fromStdString(comm));
    action.insert("type", QString::fromStdString(type));
    if (!pre.empty()) action.insert("verbal_pre_transition_instruction", QString::fromStdString(pre + "\n" + comm));
}

static void DumpRoundaboutEnterDescription(QJsonObject& action,
                                           const osmscout::RouteDescription::RoundaboutEnterDescriptionRef& /*roundaboutEnterDescription*/,
                                           const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription)
{
    std::string crossingWaysString;
    std::string comm;

    if (crossingWaysDescription) {
        crossingWaysString=CrossingWaysDescriptionToString(*crossingWaysDescription);
    }

    if (!crossingWaysString.empty()) {
        comm = "At crossing " + crossingWaysString + ", ";
    }

    action.insert("instruction", QString::fromStdString("Enter roundabout"));
    action.insert("type", QString("roundabout-enter"));
}

static void DumpRoundaboutLeaveDescription(QJsonObject& action,
                                           const osmscout::RouteDescription::RoundaboutLeaveDescriptionRef& roundaboutLeaveDescription,
                                           const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    int count = roundaboutLeaveDescription->GetExitCount();
    std::ostringstream ss;

    ss << "Leave roundabout (" << roundaboutLeaveDescription->GetExitCount() << ". exit)";

    if (nameDescription &&
            nameDescription->HasName()) {
        ss << " into street " << nameDescription->GetDescription() << "";
    }

    action.insert("instruction", QString::fromStdString(ss.str()));
    action.insert("type", QString("roundabout-exit"));
    action.insert("roundabout_exit_count", count);
}

static void DumpMotorwayEnterDescription(QJsonObject& action,
                                         const osmscout::RouteDescription::MotorwayEnterDescriptionRef& motorwayEnterDescription,
                                         const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription)
{
    std::string crossingWaysString;
    std::string comm;
    std::string pre;

    if (crossingWaysDescription) {
        crossingWaysString=CrossingWaysDescriptionToString(*crossingWaysDescription);
    }

    if (!crossingWaysString.empty()) {
        pre = "At crossing " + crossingWaysString;
    }

    comm = "Enter motorway";

    if (motorwayEnterDescription->GetToDescription() &&
            motorwayEnterDescription->GetToDescription()->HasName()) {
        comm += " " + motorwayEnterDescription->GetToDescription()->GetDescription() + "";
    }

    action.insert("instruction", QString::fromStdString(comm));
    action.insert("type", QString("merge"));
    if (!pre.empty()) action.insert("verbal_pre_transition_instruction", QString::fromStdString(pre + "\n" + comm));
}

static void DumpMotorwayChangeDescription(QJsonObject& action,
                                          const osmscout::RouteDescription::MotorwayChangeDescriptionRef& motorwayChangeDescription,
                                          const osmscout::RouteDescription::MotorwayJunctionDescriptionRef& motorwayJunctionDescription)
{
    std::string comm;
    std::string pre;

    if (motorwayJunctionDescription &&
            motorwayJunctionDescription->GetJunctionDescription()) {
        pre = "At";

        if (!motorwayJunctionDescription->GetJunctionDescription()->GetName().empty()) {
            pre += " " + motorwayJunctionDescription->GetJunctionDescription()->GetName() + "";

            if (!motorwayJunctionDescription->GetJunctionDescription()->GetRef().empty()) {
                pre += " (exit " + motorwayJunctionDescription->GetJunctionDescription()->GetRef() + ")";
            }
        }
    }

    comm = "Change motorway";

    if (motorwayChangeDescription->GetFromDescription() &&
            motorwayChangeDescription->GetFromDescription()->HasName()) {
        comm += " from " + motorwayChangeDescription->GetFromDescription()->GetDescription() + "";
    }

    if (motorwayChangeDescription->GetToDescription() &&
            motorwayChangeDescription->GetToDescription()->HasName()) {
        comm += " to " + motorwayChangeDescription->GetToDescription()->GetDescription() + "";
    }

    action.insert("instruction", QString::fromStdString(comm));
    action.insert("type", QString("motorway-change"));
    if (!pre.empty()) action.insert("verbal_pre_transition_instruction", QString::fromStdString(pre + "\n" + comm));
}

static void DumpMotorwayLeaveDescription(QJsonObject& action,
                                         const osmscout::RouteDescription::MotorwayLeaveDescriptionRef& motorwayLeaveDescription,
                                         const osmscout::RouteDescription::MotorwayJunctionDescriptionRef& motorwayJunctionDescription,
                                         const osmscout::RouteDescription::DirectionDescriptionRef& directionDescription,
                                         const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    std::string comm;
    std::string pre;
    std::string move;
    std::string type = "motorway-leave";

    if (motorwayJunctionDescription &&
            motorwayJunctionDescription->GetJunctionDescription()) {
        pre = "At";

        if (!motorwayJunctionDescription->GetJunctionDescription()->GetName().empty()) {
            pre += " " + motorwayJunctionDescription->GetJunctionDescription()->GetName() + "";

            if (!motorwayJunctionDescription->GetJunctionDescription()->GetRef().empty()) {
                pre += " (exit " + motorwayJunctionDescription->GetJunctionDescription()->GetRef() + ")";
            }
        }
    }

    comm = "Leave motorway";

    if (motorwayLeaveDescription->GetFromDescription() &&
            motorwayLeaveDescription->GetFromDescription()->HasName()) {
        comm += " " + motorwayLeaveDescription->GetFromDescription()->GetDescription() + "";
    }

    if (directionDescription &&
            directionDescription->GetCurve()!=osmscout::RouteDescription::DirectionDescription::slightlyLeft &&
            directionDescription->GetCurve()!=osmscout::RouteDescription::DirectionDescription::straightOn &&
            directionDescription->GetCurve()!=osmscout::RouteDescription::DirectionDescription::slightlyRight) {
        MoveToTurnCommand(directionDescription->GetCurve(), move, type);
        comm += ". " + move;
    }

    if (nameDescription &&
            nameDescription->HasName()) {
        comm += " into " + nameDescription->GetDescription() + "";
    }

    action.insert("instruction", QString::fromStdString(comm));
    action.insert("type", QString::fromStdString(type));
    if (!pre.empty()) action.insert("verbal_pre_transition_instruction", QString::fromStdString(pre + "\n" + comm));
}

static void DumpNameChangedDescription(QJsonObject& action,
                                       const osmscout::RouteDescription::NameChangedDescriptionRef& nameChangedDescription)
{
    std::string comm;

    comm = "Way changes name";
    if (nameChangedDescription->GetOriginDesccription()) {
        comm += " from ";
        comm += "" + nameChangedDescription->GetOriginDesccription()->GetDescription() + "";
    }

    comm += " to " + nameChangedDescription->GetTargetDesccription()->GetDescription() + "";
    action.insert("instruction", QString::fromStdString(comm));
    action.insert("type", QString("straight"));
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Main routing function
bool DBMaster::route(osmscout::Vehicle &vehicle, std::vector<osmscout::GeoCoord> &via, double radius, bool gpx, QByteArray &result)
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

    double speed_car = std::max(1.0, m_routing_speeds["Car"]);
    double speed_foot = std::max(0.01, m_routing_speeds["Foot"]);
    double speed_bicycle = std::max(0.1, m_routing_speeds["Bicycle"]);

    switch (vehicle)
    {
    case osmscout::vehicleFoot:
        routingProfile.ParametrizeForFoot(*typeConfig,
                                          speed_foot);
        break;
    case osmscout::vehicleBicycle:
        routingProfile.ParametrizeForBicycle(*typeConfig,
                                             speed_bicycle);
        break;
    case osmscout::vehicleCar:
        routingProfile.ParametrizeForCar(*typeConfig,
                                         m_routing_speeds,
                                         speed_car);
        break;
    }

    if (!router->CalculateRoute(routingProfile,
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

    if ( gpx )
    {
        ////////////////////////////////////////////////////
        /// AS GPX
        ////////////////////////////////////////////////////
        QTextStream output(&result, QIODevice::WriteOnly);
        output.setRealNumberPrecision(8);

        output << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" << "\n";
        output << "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"bin2gpx\" version=\"1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">" << "\n";

        output << "\t<wpt lat=\""<< via[0].GetLat() << "\" lon=\""<< via[0].GetLon() << "\">" << "\n";
        output << "\t\t<name>Start</name>" << "\n";
        output << "\t\t<fix>2d</fix>" << "\n";
        output << "\t</wpt>" << "\n";

        output << "\t<wpt lat=\""<< via[via.size()-1].GetLat() << "\" lon=\""<< via[via.size()-1].GetLon() << "\">" << "\n";
        output << "\t\t<name>Target</name>" << "\n";
        output << "\t\t<fix>2d</fix>" << "\n";
        output << "\t</wpt>" << "\n";

        output << "\t<trk>" << "\n";
        output << "\t\t<name>Route</name>" << "\n";
        output << "\t\t<trkseg>" << "\n";
        for (const auto &point : route_points)
        {
            output << "\t\t\t<trkpt lat=\""<< point.GetLat() << "\" lon=\""<< point.GetLon() <<"\">" << "\n";
            output << "\t\t\t\t<fix>2d</fix>" << "\n";
            output << "\t\t\t</trkpt>" << "\n";
        }
        output << "\t\t</trkseg>" << "\n";
        output << "\t</trk>" << "\n";
        output << "</gpx>" << "\n";
        router->Close();
        return true;
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

    /// Global variables
    rootObj.insert("units_distance", QString("kilometers"));
    rootObj.insert("units_time", QString("seconds"));
    rootObj.insert("language", QString("en-US"));

    {   /// locations used to calculate the route
        QJsonArray locations;
        for (const osmscout::GeoCoord &p: via)
        {
            QJsonObject po;
            po.insert("lat", p.GetLat());
            po.insert("lng", p.GetLon());
            locations.append(po);
        }

        rootObj.insert("locations", locations);
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

    ////////////////////////////////////////////////////////////////////////
    /// Route description in maneuvers
    std::list<osmscout::RouteDescription::Node>::const_iterator prevNode=description.Nodes().end();

    double totalDistance = 0;
    double totalTime = 0;
    QJsonArray maneuvers;
    QJsonObject action_previous; // keeps previous action that will be inserted later into maneuvers
    for (std::list<osmscout::RouteDescription::Node>::const_iterator node=description.Nodes().begin();
         node!=description.Nodes().end();
         ++node)
    {
        osmscout::RouteDescription::DescriptionRef                 desc;
        osmscout::RouteDescription::NameDescriptionRef             nameDescription;
        osmscout::RouteDescription::DirectionDescriptionRef        directionDescription;
        osmscout::RouteDescription::NameChangedDescriptionRef      nameChangedDescription;
        osmscout::RouteDescription::CrossingWaysDescriptionRef     crossingWaysDescription;

        osmscout::RouteDescription::StartDescriptionRef            startDescription;
        osmscout::RouteDescription::TargetDescriptionRef           targetDescription;
        osmscout::RouteDescription::TurnDescriptionRef             turnDescription;
        osmscout::RouteDescription::RoundaboutEnterDescriptionRef  roundaboutEnterDescription;
        osmscout::RouteDescription::RoundaboutLeaveDescriptionRef  roundaboutLeaveDescription;
        osmscout::RouteDescription::MotorwayEnterDescriptionRef    motorwayEnterDescription;
        osmscout::RouteDescription::MotorwayChangeDescriptionRef   motorwayChangeDescription;
        osmscout::RouteDescription::MotorwayLeaveDescriptionRef    motorwayLeaveDescription;
        osmscout::RouteDescription::MotorwayJunctionDescriptionRef motorwayJunctionDescription;

        desc=node->GetDescription(osmscout::RouteDescription::WAY_NAME_DESC);
        if (desc) {
            nameDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::NameDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::DIRECTION_DESC);
        if (desc) {
            directionDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::DirectionDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::WAY_NAME_CHANGED_DESC);
        if (desc) {
            nameChangedDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::NameChangedDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::CROSSING_WAYS_DESC);
        if (desc) {
            crossingWaysDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::CrossingWaysDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::NODE_START_DESC);
        if (desc) {
            startDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::StartDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::NODE_TARGET_DESC);
        if (desc) {
            targetDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::TargetDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::TURN_DESC);
        if (desc) {
            turnDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::TurnDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::ROUNDABOUT_ENTER_DESC);
        if (desc) {
            roundaboutEnterDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::RoundaboutEnterDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::ROUNDABOUT_LEAVE_DESC);
        if (desc) {
            roundaboutLeaveDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::RoundaboutLeaveDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::MOTORWAY_ENTER_DESC);
        if (desc) {
            motorwayEnterDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::MotorwayEnterDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::MOTORWAY_CHANGE_DESC);
        if (desc) {
            motorwayChangeDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::MotorwayChangeDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::MOTORWAY_LEAVE_DESC);
        if (desc) {
            motorwayLeaveDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::MotorwayLeaveDescription>(desc);
        }

        desc=node->GetDescription(osmscout::RouteDescription::MOTORWAY_JUNCTION_DESC);
        if (desc) {
            motorwayJunctionDescription=std::dynamic_pointer_cast<osmscout::RouteDescription::MotorwayJunctionDescription>(desc);
        }

        if (crossingWaysDescription &&
                roundaboutCrossingCounter>0 &&
                crossingWaysDescription->GetExitCount()>1) {
            roundaboutCrossingCounter+=crossingWaysDescription->GetExitCount()-1;
        }

        if (!HasRelevantDescriptions(*node)) {
            continue;
        }

        // on the exit from the loop, these values would correspond to the last node
        totalDistance = node->GetDistance();
        totalTime = H2S(node->GetTime());

        QJsonObject action; // holds a manoeuvre description
        action.insert("lat", node->GetLocation().GetLat());
        action.insert("lng", node->GetLocation().GetLon());

        if (prevNode!=description.Nodes().end() && node->GetDistance()-prevNode->GetDistance()!=0.0)
        {
            action_previous.insert("length", node->GetDistance()-prevNode->GetDistance());
            action_previous.insert("time", H2S(node->GetTime()-prevNode->GetTime()));

            // insert previous action
            maneuvers.append(action_previous);
        }

        if (startDescription) {
            DumpStartDescription(action,
                                 startDescription,
                                 nameDescription);
        }
        else if (targetDescription) {
            DumpTargetDescription(action,targetDescription);
        }
        else if (turnDescription) {
            DumpTurnDescription(action,
                                turnDescription,
                                crossingWaysDescription,
                                directionDescription,
                                nameDescription);
        }
        else if (roundaboutEnterDescription) {
            DumpRoundaboutEnterDescription(action,
                                           roundaboutEnterDescription,
                                           crossingWaysDescription);

            roundaboutCrossingCounter=1;
        }
        else if (roundaboutLeaveDescription) {
            DumpRoundaboutLeaveDescription(action,
                                           roundaboutLeaveDescription,
                                           nameDescription);

            roundaboutCrossingCounter=0;
        }
        else if (motorwayEnterDescription) {
            DumpMotorwayEnterDescription(action,
                                         motorwayEnterDescription,
                                         crossingWaysDescription);
        }
        else if (motorwayChangeDescription) {
            DumpMotorwayChangeDescription(action,
                                          motorwayChangeDescription,
                                          motorwayJunctionDescription);
        }
        else if (motorwayLeaveDescription) {
            DumpMotorwayLeaveDescription(action,
                                         motorwayLeaveDescription,
                                         motorwayJunctionDescription,
                                         directionDescription,
                                         nameDescription);
        }
        else if (nameChangedDescription) {
            DumpNameChangedDescription(action,
                                       nameChangedDescription);
        }

        prevNode=node;
        action_previous = action;
    }

    if (action_previous.value("type").toString() == "destination" &&
            route_points.size() > 0)
    {
        // correct coordinates to correspond to the last point
        // this should not be needed after fixing https://github.com/rinigus/osmscout-server/issues/30
        const osmscout::Point &p = route_points.back();
        action_previous.insert("lat", p.GetLat());
        action_previous.insert("lng", p.GetLon());

        action_previous.insert("time", 0.0);
        action_previous.insert("length", 0.0);
    }

    maneuvers.append(action_previous);

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// DONE
    router->Close();

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// SAVE RESULTS

    rootObj.insert("maneuvers", maneuvers);

    QJsonObject summary;
    summary.insert("time", totalTime);
    summary.insert("length", totalDistance);
    rootObj.insert("summary", summary);

    QJsonDocument document(rootObj);
    result = document.toJson();

    return true;
}
