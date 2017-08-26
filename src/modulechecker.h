#ifndef MODULECHECKER_H
#define MODULECHECKER_H

#include <QObject>

/// \brief Checks availability of the installed modules
///
class ModuleChecker : public QObject
{
  Q_OBJECT

  /// \brief Indicates whether fonts module has been installed
  Q_PROPERTY(bool fonts READ fonts)

  Q_PROPERTY(bool valhallaRoute READ valhallaRoute)

public:
  explicit ModuleChecker(QObject *parent = 0);

  bool fonts() const { return m_fonts; }
  bool valhallaRoute() const { return m_valhalla_route; }

signals:

public slots:

protected:
  bool m_fonts{false};
  bool m_valhalla_route{false};
};

#endif // MODULECHECKER_H
