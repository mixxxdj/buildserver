DEPRECATED
==========

This repository contains instructions and script files for building Mixxx's
dependencies and producing a Mixxx build server. This was used for Mixxx =< 2.3.
Mixxx's dependencies are now built with vcpkg from our
[vcpkg repository](https://github.com/mixxxdj/vcpkg) (vcpkg was first used for
Mixxx 2.3 on Windows, but Mixxx 2.3 still used the scripts in this repository
for macOS).

Build Server Architecture
=========================

Our build environment consists of a master machine and worker machines that
compile and package Mixxx. The master and workers need not be on the same
machine. However, we typically run the master as a "real" host machine and the
workers as virtualized guests on that machine.

* To setup the master environment, see [README.Master.md](README.Master.md).
* To setup an Ubuntu builder, see [README.Ubuntu.worker.md](README.Ubuntu.worker.md).
* To setup an OS X builder, see [README.macOS.worker.md](README.macOS.worker.md).
* To setup a Windows builder, see the [`2.3.x-windows` branch](https://github.com/mixxxdj/buildserver/tree/2.3.x-windows/).

Standalone Build Environment
============================

If you're looking to build Mixxx on your own computer, you don't need this
repository. You can follow instructions on our wiki for each platform:

* [Compiling on Linux](https://github.com/mixxxdj/mixxx/wiki/compiling-on-linux)
* [Compiling on Mac OS X](https://github.com/mixxxdj/mixxx/wiki/Compiling-on-macOS)
* [Compiling on Windows](https://github.com/mixxxdj/mixxx/wiki/compiling-on-windows)

Pre-built Environments
======================

Pre-built environments are available for Windows and macOS at:
* [Windows 2.3.x pre-built environments](https://downloads.mixxx.org/builds/buildserver/2.3.x-windows/?C=M;O=D)
* [macOS 2.3.x pre-built environments](https://downloads.mixxx.org/builds/buildserver/2.3.x-unix/?C=M;O=D)

To see which environment Mixxx 2.3.x builds are currently using, check
[packaging/macos/build_environment](https://github.com/mixxxdj/mixxx/tree/2.3/packaging/macos/build_environment)
and
[packaging/windows/build_environment](https://github.com/mixxxdj/mixxx/tree/2.3/packaging/windows/build_environment)
files in the Mixxx source tree.
