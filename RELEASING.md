# Releasing new version

Update translations on desktop:

```
lupdate osmscout-server.pro -ts translations/osmscout-server.ts
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
emacs rpm/harbour-osmscout-server.spec rpm/harbour-osmscout-server.yaml rpm/harbour-osmscout-server.changes
emacs osmscout-server.pro packaging/osmscout-server.appdata.xml packaging/ubports/manifest.json
```

Commit version changes
```
git add osmscout-server.pro packaging/osmscout-server.appdata.xml rpm/harbour-osmscout-server.changes rpm/harbour-osmscout-server.spec rpm/harbour-osmscout-server.yaml packaging/ubports/manifest.json
git status
git commit -m "bump version"
```