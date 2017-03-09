#include "filedownloader.h"

#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include <QDebug>

#include <iostream> // for a rarely expected error message

FileDownloader::FileDownloader(QNetworkAccessManager *manager,
                               QString url, QString path,
                               const Type mode,
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

  m_file.setFileName(m_path + ".download");
  if (!m_file.open(QIODevice::WriteOnly))
    {
      m_isok = false;
      return;
    }

  // start data processor if requested
  QString command;
  QStringList arguments;
  if (mode == BZ2)
    {
      command = "bunzip2";
      arguments << "-c";
    }
  else if (mode == Plain)
    {
      // nothing to do
    }
  else
    {
      std::cerr << "FileDownloader: unknown mode: " << mode << std::endl;
      m_isok = false;
      return;
    }

  if (!command.isEmpty())
    {
      m_pipe_to_process = true;
      m_process = new QProcess(this);

      connect( m_process, &QProcess::started,
               this, &FileDownloader::onProcessStarted );

      connect( m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
               this, &FileDownloader::onProcessStopped );

      connect( m_process, &QProcess::stateChanged,
               this, &FileDownloader::onProcessStateChanged );

      connect( m_process, &QProcess::readyReadStandardOutput,
               this, &FileDownloader::onProcessRead );

      connect( m_process, &QProcess::readyReadStandardError,
               this, &FileDownloader::onProcessReadError );

      m_process->start(command, arguments);
    }

  m_download_last_read_time.start();

  // start download
  startDownload();
  startTimer(const_download_timeout*1000);
}

FileDownloader::~FileDownloader()
{
  if (m_reply) m_reply->deleteLater();
  if (m_process) m_process->deleteLater();
}

void FileDownloader::startDownload()
{
  // start download
  QNetworkRequest request(m_url);
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    QString("OSM Scout Server ") + APP_VERSION);

  if (m_downloaded > 0)
    {
      QByteArray range_header = "bytes=" + QByteArray::number((qulonglong)m_downloaded) + "-";
      request.setRawHeader("Range",range_header);
    }

  m_reply = m_manager->get(request);

  connect(m_reply, SIGNAL(readyRead()),
          this, SLOT(onNetworkReadyRead()));
  connect(m_reply, SIGNAL(finished()),
          this, SLOT(onDownloaded()));
  connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(onNetworkError(QNetworkReply::NetworkError)));

  m_download_last_read_time.restart();
}

void FileDownloader::onFinished()
{
  m_file.close();

  { // delete the file if it exists already to update it
    // with a new copy
    QFile ftmp(m_path);
    ftmp.remove();
  }

  m_file.rename(m_path);

  if (m_process)
    {
      m_process->deleteLater();
      m_process = nullptr;
    }

  if (m_reply)
    {
      m_reply->deleteLater();
      m_reply = nullptr;
    }

  emit finished(m_path);
}

void FileDownloader::onError(const QString &err)
{
  m_file.close();
  m_file.remove();

  if (m_process)
    {
      m_process->deleteLater();
      m_process = nullptr;
    }

  if (m_reply)
    {
      m_reply->deleteLater();
      m_reply = nullptr;
    }

  m_isok = false;
  emit error(err);
}

void FileDownloader::onNetworkReadyRead()
{
  if (!m_reply ||
      (m_pipe_to_process && !m_process_started) ) // too early, haven't started yet
    return;

  QByteArray data_current = m_reply->readAll();
  m_cache_current.append(data_current);
  m_downloaded_gui += data_current.size();

  emit downloadedBytes(m_downloaded_gui);
  m_download_last_read_time.restart();

  // check if caches are full or whether they have to be
  // filled before writing to file/process
  if (!m_clear_all_caches && m_cache_current.size() < const_cache_size_before_swap)
    return;

  QByteArray data(m_cache_safe);
  if (m_clear_all_caches)
    {
      data.append(m_cache_current);
      m_cache_current.clear();
      m_cache_safe.clear();
    }
  else
    {
      m_cache_safe = m_cache_current;
      m_cache_current.clear();
    }

  m_downloaded += data.size();

  if (m_pipe_to_process)
    {
      m_process->write(data);
    }
  else
    {
      m_file.write(data);
      emit writtenBytes(m_downloaded);
    }
}

void FileDownloader::onDownloaded()
{
  if (!m_reply) return; // happens on error, after error cleanup and initiating retry

  if (m_reply->error() != QNetworkReply::NoError)
    return;

  if (m_pipe_to_process && !m_process_started)
    return;

  m_clear_all_caches = true;
  onNetworkReadyRead(); // update all data if needed

  if (m_pipe_to_process && m_process)
    m_process->closeWriteChannel();

  if (m_reply) m_reply->deleteLater();
  m_reply = nullptr;

  if (!m_pipe_to_process) onFinished();
}

bool FileDownloader::restartDownload()
{
  // check if we should retry before cancelling all with an error
  // this check is performed only if we managed to get some data
  if (m_downloaded_last_error != m_downloaded)
    m_download_retries = 0;

  if (m_reply &&
      m_download_retries < const_max_download_retries &&
      m_downloaded > 0 )
    {
      m_cache_safe.clear();
      m_cache_current.clear();
      m_reply->deleteLater();
      m_reply = nullptr;

      QTimer::singleShot(const_download_retry_sleep_time * 1e3,
                         this, SLOT(startDownload()));

      m_download_retries++;
      m_downloaded_gui = m_downloaded;
      m_downloaded_last_error = m_downloaded;

      return true;
    }

  return false;
}

void FileDownloader::onNetworkError(QNetworkReply::NetworkError /*code*/)
{
  if (restartDownload()) return;

  QString err = tr("Failed to download") + "<br>" + m_path +
      "<br><br>" +tr("Error code: %1").arg(QString::number(m_reply->error())) + "<br><blockquote><small>" +
      m_reply->errorString() +
      "</small></blockquote>";

  onError(err);
}

void FileDownloader::timerEvent(QTimerEvent * /*event*/)
{
  if (m_download_last_read_time.elapsed()*1e-3 > const_download_timeout)
    {
      if (restartDownload()) return;

      QString err = tr("Failed to download") + "<br>" + m_path +
          "<br><br>" +tr("Timeout");

      onError(err);
    }
}

void FileDownloader::onProcessStarted()
{
  m_process_started = true;
  onNetworkReadyRead(); // pipe all data in that has been collected already
}

void FileDownloader::onProcessRead()
{
  if (!m_process) return;

  QByteArray data = m_process->readAllStandardOutput();
  m_file.write(data);
  m_written += data.size();
  emit writtenBytes(m_written);
}

void FileDownloader::onProcessStopped(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
  if (exitCode != 0)
    {
      QString err = tr("Error in processing downloaded data");
      m_isok = false;
      emit error(err);
      return;
    }

  if (!m_process) return;

  onProcessRead();
  onFinished();
}

void FileDownloader::onProcessReadError()
{
  if (!m_process) return;

  QByteArray data = m_process->readAllStandardError();
  if (data.size() > 0)
    {
      QString
          err = tr("Error in processing downloaded data") + ": " +
          QString(data.data());
      onError(err);
    }
}

void FileDownloader::onProcessStateChanged(QProcess::ProcessState state)
{
  if ( !m_process_started && state == QProcess::NotRunning )
    {
      QString err = tr("Error in processing downloaded data: could not start the program") + " " + m_process->program();
      onError(err);
    }
}
