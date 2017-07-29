#include <iostream>

#include <valhalla/baldr/tilehierarchy.h>
#include <valhalla/baldr/graphtile.h>

using namespace valhalla;

int main()
{
  const double tol = 1e-3;
 
  for (int hier=0; hier < 2; hier++)
    {
      double step = 4;
      if (hier > 0) step = 1;
      for (double lat=-90; lat < 90-tol; lat += step)
        for (double lng=-180; lng < 180-tol; lng += step)
          {
            midgard::AABB2<midgard::PointLL> bbox(lng+tol, lat+tol, lng+step-tol, lat+step-tol);

            std::vector<baldr::GraphId> ids;
            if (hier)
              {
                for (int i=1; i <= 2; ++i)
                  {
                    std::vector<baldr::GraphId> a = baldr::TileHierarchy::GetGraphIds(bbox, i);
                    ids.insert( ids.end(), a.begin(), a.end() );
                  }
              }
            else
              ids = baldr::TileHierarchy::GetGraphIds(bbox, 0);
            
            std::cout << lng << " " << lat << " " << lng+step << " " << lat+step << " : ";

            for (const baldr::GraphId &gid: ids)
              {
                std::cout << baldr::GraphTile::FileSuffix(gid) << " ";
              }

            std::cout << "\n";
          }
    }

  return 0;
}
