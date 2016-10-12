#ifndef INFOHUB_H
#define INFOHUB_H

#include <QObject>
#include <QMutex>
#include <QString>

class InfoHub : public QObject
{
    Q_OBJECT

    //////////////////////////////////////////////////////////
    /// when true, an error has been detected that has to be fixed
    /// before the server can proceed with its work. For example,
    /// database is missing
    Q_PROPERTY(bool error READ error NOTIFY errorChanged)

public:
    explicit InfoHub(QObject *parent = 0);

    static void setError(bool e);
    bool error();

    static void logInfo(const QString &txt, bool force=false);
    static void logWarning(const QString &txt);
    static void logError(const QString &txt);

signals:
    void errorChanged(bool);

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

protected:
    QMutex m_mutex;
    bool m_error = false;
    bool m_log_info = true;
};

#endif // INFOHUB_H
