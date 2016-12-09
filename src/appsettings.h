#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>
#include <QStringList>

/// \brief A wrapper around QSettings allowing to expose it to QML
///
class AppSettings : public QSettings
{
    Q_OBJECT

public:
    AppSettings();
    virtual ~AppSettings() {}

    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);

    Q_INVOKABLE int valueInt(const QString &key);
    Q_INVOKABLE double valueFloat(const QString &key);
    Q_INVOKABLE QString valueString(const QString &key);

    Q_INVOKABLE int unitIndex() const; ///< Index of selected units
    Q_INVOKABLE int unitDisplayDecimals() const;
    Q_INVOKABLE double unitFactor() const; /// multiply by this factor when displaying distance or speed to the user

    Q_INVOKABLE bool hasUnits(const QString &key) const;
    Q_INVOKABLE QString unitName(const QString &key) const;

    void initDefaults(); ///< Initialize settings for configurable parameters on the first start

signals:
    void osmScoutSettingsChanged();

public slots:
    void fireOsmScoutSettingsChanged();

protected:
    QString unitName(bool speed) const;

protected:
    bool m_signal_osm_scout_changed_waiting = false;
};

#endif // APPSETTINGS_H
