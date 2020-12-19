# Ubuntu Touch Packaging

## Preparations

Install [Clickable](http://clickable.bhdouglass.com/en/latest/install.html)
which is used to build and publish click packages.

Create a symlink to the config file in order to omit the `-c` flag in all
clickable calls:

    ln -s packaging/click/clickable.json clickable.json

## Shortcut
To download and build dependencies as well as building click packages for all
architectures, run:

    packaging/click/build-all.sh


The click packages can be found in **click_release**.

Otherwise, follow the instructions below.

## Dependencies

**WARNING**: Dependencies may take hours to build.

Run the following command to download and compile the app dependencies:

    clickable prepare-deps
    clickable build-libs --arch armhf # for armhf devices
    clickable build-libs --arch arm64 # for arm64 devices
    clickable build-libs --arch amd64 # for desktop mode

## Building

Build the app by running

    clickable build --arch armhf # for armhf devices
    clickable build --arch arm64 # for arm64 devices
    clickable build --arch amd64 # for desktop mode

## Debugging

To debug on a Ubuntu Touch device run

    clickable --arch arm64 # or armhf, implies a clean build run, installing on device and launching
    clickable logs # to watch logs

To debug in desktop mode run one of these:

    clickable desktop # implies a clean build and run
    clickable desktop --dirty # avoid cleaning before build
    clickable desktop --skip-build # start app without rebuilding

See [Clickable docs](http://clickable.bhdouglass.com/en/latest/) for details.
