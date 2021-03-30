# Releasing new version

Update translations on desktop:

```
lupdate osmscout-server.pro server/server.pro gui/gui.pro -ts translations/osmscout-server.ts
```

Push source changes to Transifex and pull new translations

```
tx push -s
tx pull -a --minimum-perc=60
```

Commit all translations

```
git add translations/*ts
git status
git commit -m "update translations"
```

Bump version and write changelog

```
emacs rpm/harbour-osmscout-server.spec rpm/harbour-osmscout-server.changes
emacs common.pri packaging/osmscout-server.appdata.xml packaging/click/manifest.json
```

Commit version changes
```
git add osmscout-server.pro packaging/osmscout-server.appdata.xml rpm/harbour-osmscout-server.changes rpm/harbour-osmscout-server.spec rpm/harbour-osmscout-server.spec packaging/click/manifest.json common.pri
git status
git commit -m "bump version"
```

Generate vendored release

```
OSMSC_VERSION=2.0.1
git-archive-all -v --prefix=osmscout-server-${OSMSC_VERSION} osmscout-server-${OSMSC_VERSION}.tar.gz
```