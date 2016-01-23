This repository contains everything needed to make a complete buildserver for Mixxx.
By complete, we mean a buildserver able to build for all three target OS.
It contains mainly documentation and script files

Buildserver architecture
========================

Our buildserver is a virtualized environment.
The "master" server is the physical virtualization host which runs the "guest" slaves, one for each target OS.

* To make a master virtualized environment containing all three build environments, see [README.Master.md](README.Master.md)
* To make an Ubuntu slave build server, see [README.Ubuntu.slave.md](README.Ubuntu.slave.md)
* For a MAC OS X slave build server, take a look at [README.MacOSX.slave.md](README.MacOSX.slave.md)
* All windows stuff are in the [`windows_environment` branch](/mixxxdj/buildserver/tree/windows_environment)

Standalone Build servers
========================

You can also make a standalone build server that will build Mixxx for one target OS only.
Standalone build server documentation is hosted on Mixxx wiki at the moment

* [Standalone build server on Linux](http://www.mixxx.org/wiki/doku.php/compiling_on_linux)
* [Standalone build server on MacOSX] (http://www.mixxx.org/wiki/doku.php/compiling_on_os_x)
* [Standalone build server on Windows] (http://www.mixxx.org/wiki/doku.php/compiling_on_windows)
