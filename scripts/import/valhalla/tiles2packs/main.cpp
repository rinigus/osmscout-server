#include <iostream>
#include <sstream>
#include <deque>

#include <valhalla/baldr/tilehierarchy.h>
#include <valhalla/baldr/graphtile.h>

using namespace valhalla;

const double tol = 1e-3;

std::string leaf(std::vector<int> levels, double lng, double lat, std::deque<double> steps)
{
  std::ostringstream ss;
  double step = steps.front();
  steps.pop_front();

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

  if (!steps.empty())
    {
      ss << ",\n \"children\": [ ";
      std::ostringstream s1;
      double nstep = steps.front();
      for (double nlat=lat; nlat < lat+step-tol; nlat += nstep)
        for (double nlng=lng; nlng < lng+step-tol; nlng += nstep)
          s1 << leaf(levels, nlng, nlat, steps) << ", ";
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
      std::vector<int> levels;
      std::deque<double> steps;
      if (hier > 0) {
        steps = {36,12,4,2,1};
        levels = {1,2};
      }
      else {
        steps = {36,12,4};
        levels = {0};
      }

      double step = steps.front();
      for (double lat=-90; lat < 90-tol; lat += step)
        for (double lng=-180; lng < 180-tol; lng += step)
          {
            ss << leaf(levels, lng, lat, steps) << ",\n";
          }
    }

  std::string s = ss.str();
  std::cout << s.substr(0, s.length()-2) << " ]\n";

  return 0;
}
