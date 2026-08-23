# Installation instructions

## Distributions

OSM Scout Server can be installed using the following channels:

* Sailfish OS: https://openrepos.net/content/rinigus/osm-scout-server
* Desktop Linux through Flathub: https://flathub.org/apps/details/io.github.rinigus.OSMScoutServer

## Compilation from source

The server can be used as a console, a Sailfish, Kirigami, or a
QtQuick application. Corresponding configuration is selected by the `FLAVOR`
CMake configuration option. Supported values are `silica`, `kirigami`,
`qtcontrols`, `uuitk`, and `lomiri`.

When selecting a flavor for PC, it is recommended to use Kirigami instead of QtControls.

For desktop Qt 5 development, use the Podman-based development container
documented in [`.devcontainer/README.md`](.devcontainer/README.md). It provides
the supported host-independent toolchain and the `debug-kirigami-qt5` CMake
preset.

To compile on host, see devcontainer setup and packaging scripts to see how to
install the server with its dependencies.
