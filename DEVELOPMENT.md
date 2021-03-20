# Random notes on development of the server

## Generation of DBus proxies

Use `qdbusxml2cpp` to generate preliminary proxy code and adjust is as
needed. Example below:

1. Start osmscout-server

2. Pull XML description of the interface of interest and keep only node
of interest:

```
dbus-send --print-reply --dest=org.osm.scout.server1 /org/osm/scout/server1/settings org.freedesktop.DBus.Introspectable.Introspect > i.xml
emacs i.xml
```

3. Generate proxy class:

```
qdbusxml2cpp -c AppSettings -p appsettings i.xml
```

Few notes regarding generated proxy.

It is missing notification signals for properties. So, signals have to
be connected manually.

Methods tend to return `QDBusPendingReply` as a result of async
call. QML is not aware of what to do with this type. So, all calls and
return values have to be transferred to sync versions.

When method is using QDBusVariant as an argument, automatically
generated code has to be adjusted. Example of working code:

```C++
    inline void setValue(const QString &key, const QVariant &value)
    {
        QDBusVariant v(value);
        QList<QVariant> argumentList;
        argumentList << key << QVariant::fromValue(v);
        asyncCallWithArgumentList(QStringLiteral("setValue"), argumentList);
    }
```

