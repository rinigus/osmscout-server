#include "consolelogger.h"
#include "infohub.h"

#include <QDebug>
#include <iostream>

extern InfoHub infoHub;

ConsoleLogger::ConsoleLogger(QObject *parent) : QObject(parent)
{
  connect( &infoHub, &InfoHub::log,
           this, &ConsoleLogger::log );
}

void ConsoleLogger::log(QString txt)
{
  std::cout << txt.toStdString() << std::endl;
}

void ConsoleLogger::onErrorMessage(QString info)
{
  std::cerr << "ERROR: " << info.toStdString() << std::endl;
}
