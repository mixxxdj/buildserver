This repository contains instructions and script files for producing a Mixxx
build server.

Build Server Architecture
=========================

Our build environment consists of a master machine and worker machines that
compile and package Mixxx. The master and workers need not be on the same
machine. However, we typically run the master as a "real" host machine and the
workers as virtualized guests on that machine.

* To setup the master environment, see [README.Master.md](README.Master.md).
* To setup an Ubuntu builder, see [README.Ubuntu.worker.md](README.Ubuntu.worker.md).
* To setup an OS X builder, see [README.MacOSX.worker.md](README.MacOSX.worker.md).
* To setup a Windows builder, see the [`windows_environment` branch](https://github.com/mixxxdj/buildserver/tree/windows_environment/).

Standalone Build Environment
============================

If you're looking to build Mixxx on your own computer, you don't need this
repository. You can follow instructions on our wiki for each platform:

* [Compiling on Linux](http://www.mixxx.org/wiki/doku.php/compiling_on_linux)
* [Compiling on Mac OS X](http://www.mixxx.org/wiki/doku.php/compiling_on_os_x)
* [Compiling on Windows](http://www.mixxx.org/wiki/doku.php/compiling_on_windows)
