# Releasing new version

Update the Transifex source translation catalog on desktop:

```
cmake -S . -B build-release -G Ninja -DFLAVOR=kirigami -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target osmscout-update-translations
```

Push source changes to Transifex and pull new translations

```
tx push -s
tx pull -a --minimum-perc=60
```

Compile the pulled catalogs:

```
cmake --build build-release --target osmscout-translations
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
emacs CMakeLists.txt packaging/osmscout-server.appdata.xml packaging/click/manifest.json
```

Commit version changes
```
git add CMakeLists.txt packaging/osmscout-server.appdata.xml rpm/harbour-osmscout-server.changes rpm/harbour-osmscout-server.spec packaging/click/manifest.json
git status
git commit -m "bump version"
```

Generate vendored release

```
OSMSC_VERSION=2.0.1
git-archive-all -v --prefix=osmscout-server-${OSMSC_VERSION} osmscout-server-${OSMSC_VERSION}.tar.gz
```

Upload it to Gitlab after making a release.
