#!/usr/bin/env bash

# make sure to get the path right
PROGNAME=`basename $0`
PROGDIR=`dirname $0`
source $PROGDIR/config.sh

sudo adduser --gecos "Mixxx Builds,,,," mixxx

sudo apt-get install bzr ack-grep emacs24-nox git htop iotop zsh build-essential git-core bzrtools subversion sshfs
sudo mkdir /opt/mixxx/
sudo chown mixxx:mixxx /opt/mixxx
sudo chmod g+rw /opt/mixxx
sudo -u mixxx mkdir vms builds keys isos
