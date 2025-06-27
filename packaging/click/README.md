# Ubuntu Touch Packaging

## Preparations

Install [Clickable](https://clickable-ut.dev/en/latest/install.html)
which is used to build and publish click packages.

Create a symlink to the config file in order to omit the `-c` flag in all
clickable calls:

    ln -s packaging/click/clickable.yaml clickable.yaml

## Shortcut

To download and build dependencies as well as building click packages for all
architectures, run:

    packaging/click/build-all.sh

The click packages can be found in **click_release**.

Otherwise, follow the instructions below.

## Ubuntu Touch Focal

To target an older Ubuntu Touch system, set the `CLICKABLE_FRAMEWORK` env var,
Ex:

    export CLICKABLE_FRAMEWORK=ubuntu-sdk-20.04

## Dependencies

Run the following command to download and compile the app dependencies:

    clickable script prepare-deps
    clickable build --libs --arch armhf # for armhf devices
    clickable build --libs --arch arm64 # for arm64 devices
    clickable build --libs --arch amd64 # for desktop mode

## Building

Build the app by running

    clickable build --arch armhf # for armhf devices
    clickable build --arch arm64 # for arm64 devices
    clickable build --arch amd64 # for desktop mode

## Debugging

To debug on a Ubuntu Touch device run

    clickable chain build install launch logs --arch arm64 # or armhf

To debug in desktop mode run one of these:

    clickable desktop # implies to build and run
    clickable desktop --clean # build clean
    clickable desktop --skip-build # start app without rebuilding

See [Clickable docs](https://clickable-ut.dev/en/latest/) for details.
