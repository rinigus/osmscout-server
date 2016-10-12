#ifndef ROLLINGLOGGER_H
#define ROLLINGLOGGER_H

#include <QObject>
#include <QString>
#include <QQueue>

class RollingLogger : public QObject
{
    Q_OBJECT

    //////////////////////////////////////////////////////////
    /// Gives the last events in a form of a QString
    Q_PROPERTY(QString log READ log NOTIFY logChanged)

public:
    explicit RollingLogger(QObject *parent = 0);

    const QString& log() const { return m_log_txt; }

signals:
    void logChanged(QString);

public slots:
    void onSettingsChanged();

protected:
    void logEntry(const QString &txt);

protected:
    int m_size_max = 20;

    QQueue<QString> m_log;
    QString m_log_txt;
};

#endif // ROLLINGLOGGER_H
