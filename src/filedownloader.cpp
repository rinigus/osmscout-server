#include "filedownloader.h"

#include <QUrl>
#include <QDir>
#include <QFileInfo>

FileDownloader::FileDownloader(QNetworkAccessManager *manager,
                               QString url, QString path,
                               const QString mode,
                               QObject *parent):
  QObject(parent),
  m_manager(manager),
  m_url(url), m_path(path)
{
  if (!m_url.isValid())
    {
      m_isok = false;
      return;
    }

  // check path and open file
  QFileInfo finfo(m_path);
  QDir dir;
  if ( !dir.mkpath(finfo.dir().absolutePath()) )
    {
      m_isok = false;
      return;
    }

  m_file.setFileName(m_path);
  if (!m_file.open(QIODevice::WriteOnly))
    {
      m_isok = false;
      return;
    }

  // start download
  QNetworkRequest request(m_url);
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    QString("OSM Scout Server ") + APP_VERSION);

  m_reply = m_manager->get(request);

  connect(m_reply, SIGNAL(readyRead()),
          this, SLOT(onNetworkReadyRead()));
  connect(m_reply, SIGNAL(finished()),
          this, SLOT(onDownloaded()));
  connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
}

FileDownloader::~FileDownloader()
{
  if (m_reply) m_reply->deleteLater();
}

void FileDownloader::onNetworkReadyRead()
{
  if (!m_reply) return;
  QByteArray data = m_reply->readAll();
  m_downloaded += data.size();

  if (m_pipe_to_process)
    {

      emit downloadedBytes(m_downloaded);
    }
  else
  {
      m_file.write(data);
      emit writtenBytes(m_downloaded);
  }
}

void FileDownloader::onDownloaded()
{
  onNetworkReadyRead(); // update all data if needed

  if (m_reply) m_reply->deleteLater();
  m_reply = nullptr;

  if (!m_pipe_to_process)
    emit finished(m_path);
}

void FileDownloader::onNetworkError(QNetworkReply::NetworkError /*code*/)
{
  QString err = tr("Failed to download") + ": " + m_path +
      " [" + m_reply->error() + "]";

  m_isok = false;
  emit error(err);
}
