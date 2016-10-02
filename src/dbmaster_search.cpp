#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"

#include <osmscout/LocationService.h>
#include <osmscout/TextSearchIndex.h>

#include <QString>
#include <QTextStream>
#include <QVector>
#include <QMap>

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


///////////////////////////////////////////////////////////////////////////////////
/// \brief storeAsJson: stores input in result as a JSON array
/// \param input vector of JSON objects
/// \param result output data array
///
void storeAsJson(const QVector< QMap<QString, QString> > &input, QByteArray &result)
{
    QTextStream output(&result, QIODevice::WriteOnly);

    output << "[\n";

    bool first = true;
    for (QVector< QMap<QString, QString> >::const_iterator iterVec = input.constBegin();
         iterVec != input.constEnd(); ++iterVec)
    {
        if (!first) output << ",\n";
        first = false;

        output << "{\n";
        for (QMap<QString,QString>::const_iterator iterObj = iterVec->constBegin();
             iterObj != iterVec->constEnd(); ++iterObj)
        {
            if (iterObj != iterVec->constBegin()) output << ",\n";
            output << "\"" << iterObj.key() << "\": " << iterObj.value();
        }
        output << "\n}";
    }

    output << "\n]";
}

////////////////////////////////////////////////////////////////////////////////////
/// Series of functions used to either decorate the value passed to JSON map or not
static QString J(QString v)
{
    return "\"" + v + "\"";
}

static QString J(std::string v)
{
    return J(QString::fromStdString(v));
}

static QString J(double v)
{
    return QString::number(v, 'f');
}

//static QString J(int v)
//{
//    return QString::number(v);
//}

//////////////////////////////////////////////////////////////////////////////
bool DBMaster::search(QString searchPattern, QByteArray &result, size_t limit)
{
    if (m_error_flag) return false;

    QMutexLocker lk(&m_mutex);

    if (!m_database->IsOpen())
        return false;

    ///////////////////////////////////////////////////////////
    /// Search by location
    ///////////////////////////////////////////////////////////

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


    QVector< QMap<QString, QString> > all_results;
    for (const osmscout::LocationSearchResult::Entry &entry : searchResult.results)
    {
        osmscout::GeoCoord coordinates;

        if (entry.adminRegion &&
                entry.location &&
                entry.address)
        {
            QMap<QString, QString> curr_result;

            QString name;
            GetObjectNameCoor(m_database, entry.address->object, name, coordinates);

            curr_result["title"] = J(GetLocation(entry) + " " + GetAddress(entry) + ", " + GetAdminRegion(entry));
            curr_result["type"] = J(name);
            curr_result["admin_region"] = J(GetAdminRegionHierachie(locationService,
                                                                    adminRegionMap,
                                                                    entry));
            curr_result["object_id"] =  J(GetObjectId(entry.address->object));
            curr_result["lng"] = J(coordinates.GetLon());
            curr_result["lat"] = J(coordinates.GetLat());

            all_results.push_back(curr_result);
        }
        else if (entry.adminRegion &&
                 entry.location)
        {
            for (const auto &object : entry.location->objects)
            {
                QMap<QString, QString> curr_result;

                QString name;
                GetObjectNameCoor(m_database, object, name, coordinates);

                curr_result["title"] = J(GetLocation(entry) + ", " + GetAdminRegion(entry));
                curr_result["type"] = J(name);
                curr_result["admin_region"] = J(GetAdminRegionHierachie(locationService,
                                                                        adminRegionMap,
                                                                        entry));
                curr_result["object_id"] =  J(GetObjectId(object));
                curr_result["lng"] = J(coordinates.GetLon());
                curr_result["lat"] = J(coordinates.GetLat());

                all_results.push_back(curr_result);
            }
        }
        else if (entry.adminRegion &&
                 entry.poi)
        {
            QMap<QString, QString> curr_result;

            QString name;
            GetObjectNameCoor(m_database, entry.poi->object, name, coordinates);

            curr_result["title"] = J(GetPOI(entry) + ", " + GetAdminRegion(entry));
            curr_result["type"] = J(name);
            curr_result["admin_region"] = J(GetAdminRegionHierachie(locationService,
                                                                    adminRegionMap,
                                                                    entry));
            curr_result["object_id"] =  J(GetObjectId(entry.poi->object));
            curr_result["lng"] = J(coordinates.GetLon());
            curr_result["lat"] = J(coordinates.GetLat());

            all_results.push_back(curr_result);
        }
        else if (entry.adminRegion)
        {
            QMap<QString, QString> curr_result;

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

            curr_result["title"] = J(GetAdminRegion(entry));
            curr_result["type"] = J(name);
            curr_result["admin_region"] = J(GetAdminRegionHierachie(locationService,
                                                                    adminRegionMap,
                                                                    entry));
            curr_result["object_id"] =  J(id);
            curr_result["lng"] = J(coordinates.GetLon());
            curr_result["lat"] = J(coordinates.GetLat());

            all_results.push_back(curr_result);
        }
    }

    ///////////////////////////////////////////////////////////
    /// Search using free text
    ///////////////////////////////////////////////////////////

    osmscout::TextSearchIndex textSearch;
    if(!textSearch.Load(m_map_dir))
    {
        std::cout << "ERROR: Failed to load text files!"
                     "(are you sure you passed the right path?)"
                  << std::endl;
        return false;
    }

    osmscout::TextSearchIndex::ResultsMap resultsTxt;
    textSearch.Search(searchPattern.toStdString(), true, true, true, true, resultsTxt);

    osmscout::TextSearchIndex::ResultsMap::iterator it;
    size_t count = 0;
    for(it=resultsTxt.begin(); it != resultsTxt.end() && count < limit; ++it, ++count)
    {
        std::vector<osmscout::ObjectFileRef> &refs=it->second;
        std::size_t maxPrintedOffsets=5;
        std::size_t minRefCount=std::min(refs.size(),maxPrintedOffsets);

        for(size_t r=0; r < minRefCount; r++)
        {
            QMap<QString, QString> curr_result;

            QString name;
            osmscout::GeoCoord coordinates;
            GetObjectNameCoor(m_database, refs[r], name, coordinates);

            curr_result["title"] = J(it->first);
            curr_result["type"] = J(name);
            curr_result["object_id"] =  J(GetObjectId(refs[r]));
            curr_result["lng"] = J(coordinates.GetLon());
            curr_result["lat"] = J(coordinates.GetLat());

            all_results.push_back(curr_result);
        }
    }

    ////////////////////////////////////////////////////////////
    storeAsJson(all_results, result);

    return true;
}
