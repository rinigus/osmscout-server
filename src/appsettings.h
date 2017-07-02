#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>

/// \brief A wrapper around QSettings allowing to expose it to QML
///
class AppSettings : public QSettings
{
    Q_OBJECT

  /// \brief true when profiles are used to select backends
  Q_PROPERTY(bool profilesUsed READ profilesUsed NOTIFY profilesUsedChanged)

  /// \brief true if the application has been run the first time
  Q_PROPERTY(bool firstTime READ firstTime)

public:
    AppSettings();
    virtual ~AppSettings() {}

    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);

    Q_INVOKABLE int valueInt(const QString &key);
    Q_INVOKABLE bool valueBool(const QString &key);
    Q_INVOKABLE double valueFloat(const QString &key);
    Q_INVOKABLE QString valueString(const QString &key);

    Q_INVOKABLE int unitIndex() const; ///< Index of selected units
    Q_INVOKABLE int unitDisplayDecimals() const;
    Q_INVOKABLE double unitFactor() const; /// multiply by this factor when displaying distance or speed to the user

    Q_INVOKABLE bool hasUnits(const QString &key) const;
    Q_INVOKABLE QString unitName(const QString &key) const;

    void initDefaults(); ///< Initialize settings for configurable parameters on the first start

    bool profilesUsed() const { return m_profiles_used; }

    bool firstTime() const { return m_first_time; }

signals:
    void osmScoutSettingsChanged();
    void profilesUsedChanged(bool used);

public slots:
    void fireOsmScoutSettingsChanged();

protected:
    QString unitName(bool speed) const;

    void setProfile();

protected:
    bool m_signal_osm_scout_changed_waiting = false;
    bool m_profiles_used = false;
    bool m_first_time = false;
};

#endif // APPSETTINGS_H
