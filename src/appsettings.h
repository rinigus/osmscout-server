#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>

/// \brief A wrapper around QSettings allowing to expose it to QML
///
class AppSettings : public QSettings
{
    Q_OBJECT
public:
    AppSettings();
    virtual ~AppSettings() {}

    Q_INVOKABLE void setValue(const QString &key, const QVariant &value)
    { QSettings::setValue(key, value); }

    Q_INVOKABLE inline QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const
    { return QSettings::value(key, defaultValue); }

    void initDefaults(); ///< Used to initialize settings for some parameters on the first start
};

#endif // APPSETTINGS_H
