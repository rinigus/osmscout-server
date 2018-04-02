#ifndef MODULECHECKER_H
#define MODULECHECKER_H

#include <QObject>

/// \brief Checks availability of the installed modules
///
/// If everything is fine with the module (either installed or
/// not required) then corresponding variable is set to true. If
/// the installation of the module is needed, it is set to false.
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
  void modulesChanged();

public slots:
  void onSettingsChanged();

protected:
  void checkModules();

protected:
  bool m_fonts{true};
  bool m_valhalla_route{true};
};

#endif // MODULECHECKER_H
