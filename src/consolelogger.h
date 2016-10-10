#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include <QObject>

class ConsoleLogger : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleLogger(QObject *parent = 0);

protected:
    void log(QString txt);
};

#endif // CONSOLELOGGER_H
