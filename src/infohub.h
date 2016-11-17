#ifndef INFOHUB_H
#define INFOHUB_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QFile>

class InfoHub : public QObject
{
    Q_OBJECT

    //////////////////////////////////////////////////////////
    /// when true, an error has been detected that has to be fixed
    /// before the server can proceed with its work. For example,
    /// database is missing
    Q_PROPERTY(bool error READ error NOTIFY errorChanged)

    /////////////////////////////////////////////////////////
    /// number of jobs in running queue (running and waiting)
    Q_PROPERTY(int queue READ queue NOTIFY queueChanged)

public:
    explicit InfoHub(QObject *parent = 0);

    static void setError(bool e);
    bool error();

    static void logInfo(const QString &txt, bool force=false);
    static void logWarning(const QString &txt);
    static void logError(const QString &txt);

    static void addJobToQueue();
    static void removeJobFromQueue();

    int queue();

signals:
    void errorChanged(bool);
    void queueChanged(int);

    void log(QString);
    void info(QString);
    void warning(QString);
    void error(QString);

public slots:
    void onSettingsChanged();

protected:
    void impSetError(bool e);
    void impLogInfo(const QString &txt, bool force=false);
    void impLogWarning(const QString &txt);
    void impLogError(const QString &txt);
    void sessionLog(const QString &txt);

    void implChangeQueue(int delta);

protected:
    QMutex m_mutex;
    bool m_error = false;
    bool m_log_info = true;
    bool m_log_session = false;
    int m_queue = 0;

    QFile m_log_file;
};

#endif // INFOHUB_H
