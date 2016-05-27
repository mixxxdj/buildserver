#!/usr/bin/env bash

# make sure to get the path right
PROGDIR="$( cd "$(dirname "$0")" ; pwd -P )"
source $PROGDIR/config.sh

# default-jre-headless: for running Jenkins node
# xvfb: for running tests
sudo apt-get -y install emacs build-essential git-core default-jre-headless htop iotop xvfb
git config --global user.name "Mixxx Buildbot"
git config --global user.email builds@mixxx.org

# Enable passwordless sudo
echo "WARNING: Enabling passwordless sudo acess for $USER."
sudo su -c "echo \"# $USER is given passwordless access to pbuilder and debsign for buildbot purposes.\" >> /etc/sudoers.d/mixxx-ubuntu-builder"
sudo su -c "echo \"$USER    ALL=(ALL) NOPASSWD: SETENV: /usr/bin/debsign, /usr/sbin/pbuilder\" >> /etc/sudoers.d/mixxx-ubuntu-builder"

sudo apt-get -y install pbuilder debootstrap devscripts
ln -s $PROGDIR/pbuilderrc ~/.pbuilderrc

for DIST in $DISTS; do
    for ARCH in $ARCHS; do
	sudo DIST=$DIST ARCH=$ARCH pbuilder create
    done;
done;

# Because our SConscripts are a little sloppy, the host system has to have all
# the dependencies installed even though we aren't building the package on the
# host system (it's built in the pbuilder chroots).
# TODO(rryan): Fix this mess.
sudo apt-get -y build-dep mixxx
# new dependencies which aren't in our build file yet.
sudo apt-get -y install protobuf-compiler libusb-1.0-0-dev vamp-plugin-sdk libprotobuf-dev libchromaprint-dev librubberband-dev libopus-dev libopusfile-dev libsqlite3-dev libqt4-sql-sqlite
