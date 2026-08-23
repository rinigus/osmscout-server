# Dev Container

`.devcontainer/debian-qt5` is the Qt 5 desktop development environment. It is
based on Debian 12 and is intended for Podman and VS Code Dev Containers.

Open this repository in VS Code and select **Dev Containers: Reopen in
Container**. CMake Tools selects the `debug-kirigami-qt5` preset, which writes
its build directory to `build/`, installs to `install/`, and uses the Mapnik,
Valhalla, and systemd backends.

Build the image directly with:

```sh
podman build -f .devcontainer/debian-qt5/Dockerfile -t osmscout-server-qt5 .
```

The Dev Containers setup uses `--userns=keep-id`, `remoteUser`, and
`updateRemoteUserUID`; it does not require changing host ownership. Persistent
container state is under `.devcontainer/state/` and is ignored by Git.

The desktop mounts provide X11, Wayland, a session D-Bus, PulseAudio, and GPU
devices from the host. The Qt 5 image includes both XCB and Wayland platform
plugins and defaults `QT_QPA_PLATFORM` to `wayland`. Use
`QT_QPA_PLATFORM=xcb` to force X11/Xwayland when needed.

The devcontainer publishes host `127.0.0.1:8553` to the daemon's port 8553.
Set `http-listener/host=0.0.0.0` in the daemon's settings file under
`.devcontainer/state/config/osmscout-server/` and reopen the container before
starting the daemon. Do not expose this port beyond the host without a trusted
network boundary: the HTTP API has no authentication.
