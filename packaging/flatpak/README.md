For building flatpak package, run

```
flatpak-builder --repo=../flatpak --force-clean ../build-dir packaging/flatpak/io.github.rinigus.OSMScoutServer.json
```

from the source cloned source directory. Replace repo and build-dir, if needed. For generation
of flatpak bundle file, run

```
flatpak build-bundle ../flatpak osmscout-server.flatpak io.github.rinigus.OSMScoutServer
```
