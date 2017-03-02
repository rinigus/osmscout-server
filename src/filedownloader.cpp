#include "filedownloader.h"

#include <QUrl>
#include <QDir>
#include <QFileInfo>

#include <iostream> // for a rarely expected error message

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

  m_file.setFileName(m_path + ".download");
  if (!m_file.open(QIODevice::WriteOnly))
    {
      m_isok = false;
      return;
    }

  // start data processor if requested
  QString command;
  QStringList arguments;
  if (mode == "BZ2")
    {
      command = "bunzip2";
      arguments << "-c";
    }
  else if (mode.isEmpty())
    {
      // nothing to do
    }
  else
    {
      std::cerr << "FileDownloader: unknown mode: " << mode.toStdString() << std::endl;
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
  if (m_process) m_process->deleteLater();
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

  QByteArray data = m_reply->readAll();
  m_downloaded += data.size();

  if (m_pipe_to_process)
    {
      m_process->write(data);
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
  if (m_pipe_to_process && !m_process_started)
    return;

  onNetworkReadyRead(); // update all data if needed
  if (m_pipe_to_process && m_process)
    m_process->closeWriteChannel();

  if (m_reply) m_reply->deleteLater();
  m_reply = nullptr;

  if (!m_pipe_to_process) onFinished();
}

void FileDownloader::onNetworkError(QNetworkReply::NetworkError /*code*/)
{
  QString err = tr("Failed to download") + ": " + m_path +
      " [" + QString::number(m_reply->error()) + "]";
  onError(err);
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
