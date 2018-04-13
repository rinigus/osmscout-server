#include <iostream>
#include <sstream>

#include <valhalla/baldr/tilehierarchy.h>
#include <valhalla/baldr/graphtile.h>

using namespace valhalla;

const double tol = 1e-3;

std::string leaf(std::vector<int> levels, double lng, double lat, double step, double minstep)
{
  std::ostringstream ss;

  ss << "{ \"bbox\": [ " << lng << ", " << lat << ", " << lng+step << ", " << lat+step << " ], ";

  ss << "\"files\": [ ";

  midgard::AABB2<midgard::PointLL> bbox(lng+tol, lat+tol, lng+step-tol, lat+step-tol);
  std::vector<baldr::GraphId> ids;
  for (int i: levels)
    {
      std::vector<baldr::GraphId> a = baldr::TileHierarchy::GetGraphIds(bbox, i);
      ids.insert( ids.end(), a.begin(), a.end() );
    }

  {
    std::ostringstream s1;
    for (const baldr::GraphId &gid: ids)
      s1 << "\"" << baldr::GraphTile::FileSuffix(gid) << "\", ";
    std::string s = s1.str();

    ss << s.substr(0, s.length()-2) << " ]";
  }

  double nstep = step / 2;
  if (nstep + tol > minstep)
    {
      ss << ",\n \"children\": [ ";
      std::ostringstream s1;
      for (double nlat=lat; nlat < lat+step-tol; nlat += nstep)
        for (double nlng=lng; nlng < lng+step-tol; nlng += nstep)
          s1 << leaf(levels, nlng, nlat, nstep, minstep) << ", ";
      std::string s = s1.str();
      ss << s.substr(0, s.length()-2) << " ]";
    }

  ss << " } ";
  
  return ss.str();
}

int main()
{
  std::ostringstream ss;
  ss << "[ ";

  for (int hier=0; hier < 2; hier++)
    {
      const double step = 16;
      double minstep = 4;

      std::vector<int> levels;
      if (hier > 0) {
        minstep = 1;
        levels = {1,2};
      }
      else
        levels = {0};
      
      for (double lat=-90; lat < 90-tol; lat += step)
        for (double lng=-180; lng < 180-tol; lng += step)
          {
            ss << leaf(levels, lng, lat, step, minstep) << ",\n";
          }
    }

  std::string s = ss.str();
  std::cout << s.substr(0, s.length()-2) << " ]\n";

  return 0;
}
