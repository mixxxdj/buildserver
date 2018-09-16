Host Setup
==========

These instructions require [Fabric](http://www.fabfile.org/) to run. You should
pick a prefix for Mixxx-related data (VMs, ISOs, SSH keys, etc.) to live. This
has traditionally been `/opt/mixxx`, but it can be anywhere.

```
$ MIXXX_ROOT=/opt/mixxx
$ mkdir -p $MIXXX_ROOT && cd $MIXXX_ROOT
$ sudo apt-get install git-core fabric
$ git clone https://github.com/mixxxdj/buildserver.git --depth 1 ./
$ fab setup_host
```

Machine and Directory Layout
============================

We use qemu-kvm for virtualization since it has the best support for virtualizing macOS.

All VMs are stored in `/opt/mixxx/vms`
To save time re-downloading, all ISOs for e.g. Ubuntu versions are in `/opt/mixxx/isos`

Naming Conventions
==================

Build VMs are named according to this pattern: `build-$(OS)-$(VERSION)-$(ARCH)`

For example:
```
build-ubuntu-10.04-amd64
build-windows-10-amd64
build-windows-7-amd64
build-osx-10.6-amd64
```
