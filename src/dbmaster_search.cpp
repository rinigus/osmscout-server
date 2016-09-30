#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"

#include <osmscout/LocationService.h>

#include <QString>
#include <QTextStream>

bool GetAdminRegionHierachie(const osmscout::LocationService& locationService,
                             const osmscout::AdminRegionRef& adminRegion,
                             std::map<osmscout::FileOffset,osmscout::AdminRegionRef>& adminRegionMap,
                             std::string& path)
{
    if (!locationService.ResolveAdminRegionHierachie(adminRegion,
                                                      adminRegionMap)) {
        return false;
    }

    if (!adminRegion->aliasName.empty()) {
        if (!path.empty()) {
            path.append("/");
        }

        path.append(adminRegion->aliasName);
    }

    if (!path.empty()) {
        path.append("/");
    }

    path.append(adminRegion->name);

    osmscout::FileOffset parentRegionOffset=adminRegion->parentRegionOffset;

    while (parentRegionOffset!=0) {
        std::map<osmscout::FileOffset,osmscout::AdminRegionRef>::const_iterator entry=adminRegionMap.find(parentRegionOffset);

        if (entry==adminRegionMap.end()) {
            break;
        }

        osmscout::AdminRegionRef parentRegion=entry->second;

        if (!path.empty()) {
            path.append("/");
        }

        path.append(parentRegion->name);

        parentRegionOffset=parentRegion->parentRegionOffset;
    }

    return true;
}

QString GetAddress(const osmscout::LocationSearchResult::Entry& entry)
{
    std::string label = entry.address->name;
    if ( !entry.address->postalCode.empty() )
        label += ", " + entry.address->postalCode;
    return QString::fromStdString(label);
}

QString GetLocation(const osmscout::LocationSearchResult::Entry& entry)
{
    return QString::fromStdString(entry.location->name);
}

QString GetPOI(const osmscout::LocationSearchResult::Entry& entry)
{
    return QString::fromStdString(entry.poi->name);
}

QString GetAdminRegion(const osmscout::LocationSearchResult::Entry& entry)
{
    std::string label;

    if (!entry.adminRegion->aliasName.empty()) {
        label.append(entry.adminRegion->aliasName);
    }
    else {
        label.append(entry.adminRegion->name);
    }

    return QString::fromStdString(label);
}

void GetObjectNameCoor( const osmscout::DatabaseRef& database,
                        const osmscout::ObjectFileRef& object,
                        QString &name,
                        osmscout::GeoCoord& coordinates )
{
    std::string label;

    if (object.GetType()==osmscout::RefType::refNode)
    {
        osmscout::NodeRef node;

        if (database->GetNodeByOffset(object.GetFileOffset(),
                                      node)) {
            label+=node->GetType()->GetName();
            coordinates = node->GetCoords();
        }
    }
    else if (object.GetType()==osmscout::RefType::refArea) {
        osmscout::AreaRef area;

        if (database->GetAreaByOffset(object.GetFileOffset(),
                                      area)) {
            label+=area->GetType()->GetName();
            area->GetCenter(coordinates);
        }
    }
    else if (object.GetType()==osmscout::RefType::refWay) {
        osmscout::WayRef way;

        if (database->GetWayByOffset(object.GetFileOffset(),
                                     way)) {
            label+=way->GetType()->GetName();
            way->GetCenter(coordinates);
        }
    }

    name = QString::fromStdString(label);
}


QString GetObjectId(const osmscout::ObjectFileRef& object)
{
    std::string label;

    label=object.GetTypeName();
    label+=" ";
    label+=osmscout::NumberToString(object.GetFileOffset());

    return QString::fromStdString(label);
}

QString GetAdminRegionHierachie(const osmscout::LocationService& locationService,
                                std::map<osmscout::FileOffset,osmscout::AdminRegionRef>& adminRegionMap,
                                const osmscout::LocationSearchResult::Entry& entry)
{
    std::string path;

    if (!GetAdminRegionHierachie(locationService,
                                 entry.adminRegion,
                                 adminRegionMap,
                                 path)) {
        return "";
    }

    return QString::fromStdString(path);
}


bool DBMaster::search(QString searchPattern, QByteArray &result, size_t limit)
{
    if (m_error_flag) return false;

    QMutexLocker lk(&m_mutex);

    if (!m_database->IsOpen())
        return false;

    osmscout::LocationService locationService(m_database);
    osmscout::LocationSearch search;
    osmscout::LocationSearchResult searchResult;
    std::map<osmscout::FileOffset,osmscout::AdminRegionRef> adminRegionMap;

    search.limit = limit;

    if (!locationService.InitializeLocationSearchEntries(searchPattern.toStdString(),
                                                         search))
    {
        std::cerr << "Error while parsing search string" << "\n";
        return false;
    }

    if (!locationService.SearchForLocations(search,
                                            searchResult))
    {
        std::cerr << "Error while searching for location" << "\n";
        return false;
    }

    QTextStream output(&result, QIODevice::WriteOnly);

    output << "[\n";

    bool first = true;
    for (const osmscout::LocationSearchResult::Entry &entry : searchResult.results)
    {
        osmscout::GeoCoord coordinates;

        if (!first) output << ",\n";
        if (entry.adminRegion) first = false;

        if (entry.adminRegion &&
                entry.location &&
                entry.address)
        {
            QString name;
            GetObjectNameCoor(m_database, entry.address->object, name, coordinates);

            output << "{\n";
            output << "\"title\": \"" << GetLocation(entry) << " " << GetAddress(entry) << ", " << GetAdminRegion(entry)
                   << "\",\n";
            output << "\"type\": \"" << name << "\",\n";
            output << "\"admin_region\": \""
                   << GetAdminRegionHierachie(locationService,
                                              adminRegionMap,
                                              entry) << "\",\n";
            output << "\"object_id\": \"" << GetObjectId(entry.address->object) << "\",\n";
            output << "\"lng\": " << coordinates.GetLon() << ",\n";
            output << "\"lat\": " << coordinates.GetLat() << "\n";
            output << "}\n";
        }
        else if (entry.adminRegion &&
                 entry.location)
        {
            bool first = true;
            for (const auto &object : entry.location->objects)
            {
                if (!first) output << ",\n";
                first = false;

                QString name;
                GetObjectNameCoor(m_database, object, name, coordinates);

                output << "{\n";
                output << "\"title\": \"" << GetLocation(entry) << ", " << GetAdminRegion(entry)
                       << "\",\n";
                output << "\"type\": \"" << name << "\",\n";
                output << "\"admin_region\": \""
                       << GetAdminRegionHierachie(locationService,
                                                  adminRegionMap,
                                                  entry) << "\",\n";
                output << "\"object_id\": \"" << GetObjectId(object) << "\",\n";
                output << "\"lng\": " << coordinates.GetLon() << ",\n";
                output << "\"lat\": " << coordinates.GetLat() << "\n";
                output << "}\n";
            }
        }
        else if (entry.adminRegion &&
                 entry.poi)
        {
            QString name;
            GetObjectNameCoor(m_database, entry.poi->object, name, coordinates);

            output << "{\n";
            output << "\"title\": \"" << GetPOI(entry) << ", " << GetAdminRegion(entry)
                   << "\",\n";
            output << "\"type\": \"" << name << "\",\n";
            output << "\"admin_region\": \""
                   << GetAdminRegionHierachie(locationService,
                                              adminRegionMap,
                                              entry) << "\",\n";
            output << "\"object_id\": \"" << GetObjectId(entry.poi->object) << "\",\n";
            output << "\"lng\": " << coordinates.GetLon() << ",\n";
            output << "\"lat\": " << coordinates.GetLat() << "\n";
            output << "}\n";
        }
        else if (entry.adminRegion)
        {
            QString name, id;
            if (entry.adminRegion->aliasObject.Valid())
            {
                GetObjectNameCoor(m_database, entry.adminRegion->aliasObject, name, coordinates);
                id = GetObjectId(entry.adminRegion->aliasObject);
            }
            else
            {
                GetObjectNameCoor(m_database, entry.adminRegion->object, name, coordinates);
                id = GetObjectId(entry.adminRegion->object);
            }

            output << "{\n";
            output << "\"title\": \"" << GetAdminRegion(entry)
                   << "\",\n";
            output << "\"type\": \"" << name << "\",\n";
            output << "\"admin_region\": \""
                   << GetAdminRegionHierachie(locationService,
                                              adminRegionMap,
                                              entry) << "\",\n";
            output << "\"object_id\": \"" << id << "\",\n";
            output << "\"lng\": " << coordinates.GetLon() << ",\n";
            output << "\"lat\": " << coordinates.GetLat() << "\n";
            output << "}\n";
        }
    }

    output << "]\n";

    return true;
}
