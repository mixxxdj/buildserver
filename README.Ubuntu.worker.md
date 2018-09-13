Building an Ubuntu builder
==========================

These instructions assume you have already followed the
[Host Setup](README.Master.md#host-setup) instructions and that you are
executing the instructions from your `$MIXXX_ROOT` (e.g. `/opt/mixxx`).

1. `fab make_ubuntu_builder:name=build-ubuntu-18.04-amd64`: This step downloads
   the current supported Ubuntu Server LTS release ISO, creates an unattended
   install ISO in `$CWD/isos/`, and prompts you for information about the VM to
   create (RAM, disk, cores, SSH forwarding port, VNC forwarding port). It
   creates a disk image for your VM at `$CWD/vms/build-ubuntu-18.04-amd64/`. The
   name of the VM in this example (`build-ubuntu-18.04-amd64`) follows our
   [naming conventions](README.Master.md#naming-conventions).
2. `fab run_builder:build-ubuntu-18.04-amd64`: This step starts your VM with
   QEMU KVM. On the first boot, Ubuntu Server will install automatically and
   reboot. You can monitor the progress by connecting to the VNC port you
   configured above.
3. Your Ubuntu Server guest is now running and can be SSH'd into on the host:port
   you specified in step 1 using `$CWD/keys/build-login.pub`. For example: `ssh
   -p 10014 mixxx@127.0.0.1 -i $CWD/keys/build-login`.
3. `fab -H 127.0.0.1:10014 setup_ubuntu_builder`: This step prepares the guest
   for performing Mixxx builds (install necessary packages, builds pbuilder
   chroots, etc.).
4. Copy release GPG key to ~/.gnupg on the builder machine
