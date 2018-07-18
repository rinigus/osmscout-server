#include "valhallamapmatcherdbus.h"

#include <QDebug>

ValhallaMapMatcherDBus::ValhallaMapMatcherDBus(QObject *parent):
  QDBusAbstractAdaptor(parent)
{

}

ValhallaMapMatcherDBus::~ValhallaMapMatcherDBus()
{
}

void ValhallaMapMatcherDBus::start(const QDBusMessage &message)
{
  qDebug() << "Start called: " << message;
}

//bool ValhallaMapMatcherDBus::start(int mode)
//{
//  qDebug() << "Start called: " << mode;
//  return true;
//}

//bool ValhallaMapMatcherDBus::stop(int mode)
//{
//  qDebug() << "Stop called: " << mode;
//  return true;
//}

//bool ValhallaMapMatcherDBus::stop()
//{
//  qDebug() << "Stop called";
//  return true;
//}
