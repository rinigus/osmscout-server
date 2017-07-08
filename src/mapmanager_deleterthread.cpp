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
