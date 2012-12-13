#!/usr/bin/env bash

# make sure to get the path right
PROGNAME=`basename $0`
PROGDIR=`dirname $0`
source $PROGDIR/config.sh

sudo apt-get install bzr emacs build-essential

# Enable passwordless sudo
#echo "WARNING: Enabling passwordless sudo acess for $USER."
#sudo su -c "echo \"# $USER is given passwordless access to pbuilder and debsign for buildbot purposes.\" >> /etc/sudoers"
#sudo su -c "echo \"$USER    ALL=(ALL) NOPASSWD: SETENV: /usr/bin/debsign, /usr/sbin/pbuilder\" >> /etc/sudoers"

sudo apt-get install pbuilder debootstrap devscripts
cp pbuilderrc ~/.pbuilderrc

for DIST in $DISTS; do
    for ARCH in $ARCHS; do
	sudo DIST=$DIST ARCH=$ARCH pbuilder create
    done;
done;
