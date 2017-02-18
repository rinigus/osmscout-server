#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>

/// \brief Downloads a file specified by URL
///
/// Downloads a file as specified by URL and stores in a given path.
/// If the required directories do not exist, creates all parent directories
/// as needed. If specified that the format is BZ2, the downloader pipes
/// the internet stream through a process running bunzip2.
class FileDownloader : public QObject
{
  Q_OBJECT
public:
  explicit FileDownloader(QNetworkAccessManager *manager, QString url, QString path, const QString mode = QString(), QObject *parent = 0);
  ~FileDownloader();

  operator bool() const { return m_isok; }

signals:
  void downloadedBytes(size_t sz);
  void downloadFinished();
  void writtenBytes(size_t sz);
  void finished(QString path);
  void error(QString error_text);

public slots:
  void onNetworkReadyRead();
  void onDownloaded();
  void onNetworkError(QNetworkReply::NetworkError code);

protected:
  QNetworkAccessManager *m_manager;
  QUrl m_url;
  QString m_path;

  QNetworkReply *m_reply{nullptr};

  QFile m_file;

  bool m_pipe_to_process{false};
  bool m_isok{true};

  size_t m_downloaded{0};
  size_t m_written{0};
};

#endif // FILEDOWNLOADER_H
