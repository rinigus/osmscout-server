#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <osmscout/LocationService.h>
#include <osmscout/TextSearchIndex.h>
#include <osmscout/POIService.h>

#include <QString>
#include <QTextStream>
#include <QVector>
#include <QMap>
#include <QSet>

#include <QDebug>

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
            path.append(", ");
        }

        path.append(adminRegion->aliasName);
    }

    if (!path.empty()) {
        path.append(", ");
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
            path.append(", ");
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

template <typename T>
QString GetObjectIdRef(const T& ref)
{
    std::string label;

    label=ref->GetTypeName();
    label+=" ";
    label+=osmscout::NumberToString(ref->GetFileOffset());

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

    return QString::fromStdString(path).trimmed();
}


///////////////////////////////////////////////////////////////////////////////////
/// \brief storeAsJson: stores input in result as a JSON array
/// \param input vector of JSON objects
/// \param result output stream
///
void storeAsJson(const QVector< QMap<QString, QString> > &input, QTextStream &output)
{
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
bool DBMaster::search(const QString &searchPattern, SearchResults &all_results, size_t limit)
{
    if (m_error_flag) return false;

    QMutexLocker lk(&m_mutex);

    if (!m_database->IsOpen())
    {
        InfoHub::logWarning("Database is not open, cannot search");
        return false;
    }

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
        InfoHub::logWarning("Error while parsing search string");
        return false;
    }

    if (!locationService.SearchForLocations(search,
                                            searchResult))
    {
        InfoHub::logError("Error while searching for location");
        return false;
    }


    for (const osmscout::LocationSearchResult::Entry &entry : searchResult.results)
    {
        if (all_results.length()>=limit)
            break;

        osmscout::GeoCoord coordinates;

        if (entry.adminRegion &&
                entry.location &&
                entry.address)
        {
            if ( all_results.contains( entry.address->object ) )
                continue;

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

            all_results.add(entry.address->object, curr_result);
        }
        else if (entry.adminRegion &&
                 entry.location)
        {
            for (const auto &object : entry.location->objects)
            {
                if ( all_results.contains( object ) )
                    continue;

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

                all_results.add(object, curr_result);
            }
        }
        else if (entry.adminRegion &&
                 entry.poi)
        {
            if ( all_results.contains( entry.poi->object ) )
                continue;

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

            all_results.add(entry.poi->object, curr_result);
        }
        else if (entry.adminRegion)
        {
            QMap<QString, QString> curr_result;

            QString name, id;
            osmscout::FileOffset objid;
            if (entry.adminRegion->aliasObject.Valid())
            {
                GetObjectNameCoor(m_database, entry.adminRegion->aliasObject, name, coordinates);
                id = GetObjectId(entry.adminRegion->aliasObject);
                objid = entry.adminRegion->aliasObject.GetFileOffset();
            }
            else
            {
                GetObjectNameCoor(m_database, entry.adminRegion->object, name, coordinates);
                id = GetObjectId(entry.adminRegion->object);
                objid = entry.adminRegion->object.GetFileOffset();
            }

            if (all_results.contains(objid))
                continue;

            curr_result["title"] = J(GetAdminRegion(entry));
            curr_result["type"] = J(name);
            curr_result["admin_region"] = J(GetAdminRegionHierachie(locationService,
                                                                    adminRegionMap,
                                                                    entry));
            curr_result["object_id"] =  J(id);
            curr_result["lng"] = J(coordinates.GetLon());
            curr_result["lat"] = J(coordinates.GetLat());

            all_results.add(objid, curr_result);
        }
    }

    ///////////////////////////////////////////////////////////
    /// Search using free text
    ///////////////////////////////////////////////////////////

    osmscout::TextSearchIndex textSearch;
    if(!textSearch.Load(m_map_dir))
    {
        InfoHub::logError("ERROR: Failed to load text index files, search was for locations only");
        return true; // since we were able to search for location
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
            if (all_results.length()>=limit)
                break;

            osmscout::ObjectFileRef fref = refs[r];

            if (all_results.contains(fref))
                continue;

            QMap<QString, QString> curr_result;

            QString name;
            osmscout::GeoCoord coordinates;
            GetObjectNameCoor(m_database, fref, name, coordinates);

            curr_result["title"] = J(it->first);
            curr_result["type"] = J(name);
            curr_result["object_id"] =  J(GetObjectId(fref));
            curr_result["lng"] = J(coordinates.GetLon());
            curr_result["lat"] = J(coordinates.GetLat());

            all_results.add(fref, curr_result);
        }
    }

    return true;
}


bool DBMaster::search(const QString &searchPattern, QByteArray &result, size_t limit)
{
    SearchResults all_results;
    if ( !search(searchPattern, all_results, limit) )
        return false;

    QTextStream output(&result, QIODevice::WriteOnly);
    storeAsJson(all_results.results(), output);

    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
/// Search POI
bool DBMaster::guide(const QString &poitype, double lat, double lon, double radius, size_t limit, QByteArray &result)
{
    if (m_error_flag) return false;

    QMutexLocker lk(&m_mutex);

    if (!m_database->IsOpen())
    {
        InfoHub::logWarning("Database is not open, cannot search for POI");
        return false;
    }

//    qDebug() << "guide: " << poitype << " lat=" << lat << " lon=" << lon << " radius=" << radius;

    osmscout::GeoCoord center_coordinate(lat, lon);
    osmscout::GeoBox region_box( osmscout::GeoBox::BoxByCenterAndRadius(center_coordinate, radius) );

//    qDebug() << "region: lat: " << region_box.GetMinLat() << "-" << region_box.GetMaxLat()
//             << " lon: " << region_box.GetMinLon() << "-" << region_box.GetMaxLon();

    osmscout::TypeConfigRef typeConfig(m_database->GetTypeConfig());
    osmscout::TypeInfoSet nodeTypes(*typeConfig);
    osmscout::TypeInfoSet wayTypes(*typeConfig);
    osmscout::TypeInfoSet areaTypes(*typeConfig);
    osmscout::NameFeatureLabelReader nameLabelReader(*typeConfig);

    for (const osmscout::TypeInfoRef &r: typeConfig->GetNodeTypes())
    {
        QString name = QString::fromStdString(r->GetName());
        if (name.contains(poitype, Qt::CaseInsensitive))
        {
            nodeTypes.Set(r);
//            qDebug() << "types: N " << name;
        }
    }

    for (const osmscout::TypeInfoRef &r: typeConfig->GetWayTypes())
    {
        QString name = QString::fromStdString(r->GetName());
        if (name.contains(poitype, Qt::CaseInsensitive))
        {
            wayTypes.Set(r);
//            qDebug() << "types: W " << name;
        }
    }

    for (const osmscout::TypeInfoRef &r: typeConfig->GetAreaTypes())
    {
        QString name = QString::fromStdString(r->GetName());
        if (name.contains(poitype, Qt::CaseInsensitive))
        {
            areaTypes.Set(r);
//            qDebug() << "types: A " << name;
        }
    }

    osmscout::POIService poiService(m_database);
    std::vector<osmscout::NodeRef> nodes;
    std::vector<osmscout::WayRef>  ways;
    std::vector<osmscout::AreaRef> areas;

    if (!poiService.GetPOIsInArea(region_box,
                                  nodeTypes,
                                  nodes,
                                  wayTypes,
                                  ways,
                                  areaTypes,
                                  areas))
    {
        InfoHub::logError("Cannot load data from database");
        return false;
    }

    SearchResults all_results;

    for (const osmscout::NodeRef &node: nodes)
    {
        if (all_results.length()>=limit)
            break;

        osmscout::FileOffset fref = node->GetFileOffset();

        if (all_results.contains(fref))
            continue;

        QMap<QString, QString> curr_result;

        osmscout::GeoCoord coordinates = node->GetCoords();

        curr_result["title"] = J(nameLabelReader.GetLabel((node->GetFeatureValueBuffer())));
        curr_result["type"] = J(node->GetType()->GetName());
        curr_result["object_id"] =  J("Node " + osmscout::NumberToString(fref));
        curr_result["lng"] = J(coordinates.GetLon());
        curr_result["lat"] = J(coordinates.GetLat());

        all_results.add(fref, curr_result);
    }

    for (const osmscout::WayRef &way: ways)
    {
        if (all_results.length()>=limit)
            break;

        osmscout::FileOffset fref = way->GetFileOffset();

        if (all_results.contains(fref))
            continue;

        QMap<QString, QString> curr_result;

        osmscout::GeoCoord coordinates; way->GetCenter(coordinates);

        curr_result["title"] = J(nameLabelReader.GetLabel((way->GetFeatureValueBuffer())));
        curr_result["type"] = J(way->GetType()->GetName());
        curr_result["object_id"] =  J("Way " + osmscout::NumberToString(fref));
        curr_result["lng"] = J(coordinates.GetLon());
        curr_result["lat"] = J(coordinates.GetLat());

        all_results.add(fref, curr_result);
    }

    for (const osmscout::AreaRef &area: areas)
    {
        if (all_results.length()>=limit)
            break;

        osmscout::FileOffset fref = area->GetFileOffset();

        if (all_results.contains(fref))
            continue;

        QMap<QString, QString> curr_result;

        osmscout::GeoCoord coordinates; area->GetCenter(coordinates);

        curr_result["title"] = J(nameLabelReader.GetLabel((area->GetFeatureValueBuffer())));
        curr_result["type"] = J(area->GetType()->GetName());
        curr_result["object_id"] =  J("Area " + osmscout::NumberToString(fref));
        curr_result["lng"] = J(coordinates.GetLon());
        curr_result["lat"] = J(coordinates.GetLat());

        all_results.add(fref, curr_result);
    }

    ////////////////////////////////////////////
    /// Write the results

    QTextStream output(&result, QIODevice::WriteOnly);
    output.setRealNumberPrecision(8);
    output << "{\n"
           << "\"origin\": { \"lng\": " << lon << ", \"lat\": " << lat << "},\n"
           << "\"results\": ";

    storeAsJson(all_results.results(), output);

    output << "\n}\n";

    return true;
}

bool DBMaster::guide(const QString &poitype, const QString &searchPattern, double radius, size_t limit, QByteArray &result)
{
    SearchResults all_results;
    if ( !search(searchPattern, all_results, 1) )
    {
        InfoHub::logWarning("Search for reference point failed");
        return false;
    }

    if (all_results.length() == 0)
    {
        QTextStream output(&result, QIODevice::WriteOnly);
        output << "{ }";
        return true;
    }

//    qDebug() << all_results.results().at(0);

    double lat = all_results.results().at(0)["lat"].toDouble();
    double lon = all_results.results().at(0)["lng"].toDouble();

    return guide(poitype, lat, lon, radius, limit, result);
}


bool DBMaster::poiTypes(QByteArray &result)
{
    if (m_error_flag) return false;

    QMutexLocker lk(&m_mutex);

    if (!m_database->IsOpen())
    {
        InfoHub::logWarning("Database is not open, cannot list POI types");
        return false;
    }

    QTextStream output(&result, QIODevice::WriteOnly);

    osmscout::TypeConfigRef typeConfig(m_database->GetTypeConfig());

    for (const osmscout::TypeInfoRef &r: typeConfig->GetTypes())
    {
        QString name = QString::fromStdString(r->GetName());
        output << name << "\n";
    }

    return true;
}
