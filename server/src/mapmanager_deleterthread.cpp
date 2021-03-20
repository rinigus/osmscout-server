/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mapmanager_deleterthread.h"
#include "infohub.h"

using namespace MapManager;

void DeleterThread::run()
{
  size_t n = m_todelete.size();
  size_t k = 0;

  InfoHub::logInfo(tr("Cleanup: %1 files to delete").arg(n));

  for (auto fname: m_todelete)
    {
      if ( !m_root_dir.remove(fname) )
        {
          InfoHub::logWarning(tr("Error while deleting file:") + " " + fname);
          InfoHub::logWarning(tr("Cancelling the removal of remaining files."));
          return;
        }

      n--;
      k++;

      if (k % 100 == 0)
        {
          InfoHub::logInfo(tr("Files left to remove: %1").arg(n));
          k = 0;
        }
    }

  InfoHub::logInfo(tr("Cleanup finished"));
}
