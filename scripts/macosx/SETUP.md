Detailed instructions: http://www.contrib.andrew.cmu.edu/~somlo/OSXKVM/

As of Ubuntu 14.04, need to build kvm, qemu, chameleon, etc. from master.

HOST SETUP
==========

```
export KVMOSX_PREFIX=/usr/local/kvmosx
mkdir -p $KVMOSX_PREFIX
chown mixxx:mixxx $KVMOSX_PREFIX
chmod g+rw $KVMOSX_PREFIX

# kvm
git clone git://git.kernel.org/pub/scm/virt/kvm/kvm.git
cd kvm
# master fails to build so checkout the revision that includes the author's patch
git checkout 100943c54e0947a07d2c0185368fc2fd848f7f28
git clone git://git.kiszka.org/kvm-kmod.git
cd kvm-kmod
make LINUX=../kvm clean sync all

modprobe -r kvm_intel
cp ./x86/kvm*.ko /lib/modules/`uname -a`/kernel/arch/x86/kvm/
modprobe kvm_intel

# Give mixxx user access to /dev/kvm
sudo adduser mixxx kvm
sudo chown root:kvm /dev/kvm
sudo chmod g+rw /dev/kvm

# qemu
git clone git://git.qemu.org/qemu.git;
cd qemu
./configure --prefix=$KVMOSX_PREFIX --target-list=x86_64-softmmu
make clean; make; make install

# seabios
git clone git://git.seabios.org/seabios.git
cd seabios
make
cp out/bios.bin $KVMOSX_PREFIX/lib/bios-mac.bin

# chameleon bootloader
wget http://www.contrib.andrew.cmu.edu/~somlo/OSXKVM/chameleon_svn2360_boot
cp chameleon_svn2360_boot $KVMOSX_PREFIX/lib/

# setup vm
export VM_NAME=build-macosx-10.9-amd64
export VM_PATH=/opt/mixxx/vms/$VM_NAME
mkdir $VM_PATH
cd $VM_PATH
qemu-img create -f qcow2 $VM_PATH/$VM_NAME.img 45G

# Ports for the localhost interface on the host that forward to the guest.
export GUEST_SSH=127.0.0.1:10010
export GUEST_QEMU_VNC=127.0.0.1:10
export GUEST_NATIVE_VNC=127.0.0.1:5911

$KVMOSX_PREFIX/bin/qemu-system-x86_64 -enable-kvm \
  -m 2048 -cpu core2duo -smp 2 -machine q35 \
  -vga std -usb -device usb-kbd -device usb-mouse \
  -device isa-applesmc,osk="ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc" \
  -kernel $KVMOSX_PREFIX/lib/chameleon_svn2360_boot -smbios type=2 \
  -device ide-drive,bus=ide.2,drive=MacHDD -drive id=MacHDD,if=none,file=$VM_PATH/$VM_NAME.img \
  -bios $KVMOSX_PREFIX/lib/bios-mac.bin \
  -device virtio-net,netdev=hub0port0,id=eth0 \
  -netdev user,id=hub0port0,hostfwd=tcp:${GUEST_NATIVE_VNC}-:5900,hostfwd=tcp:${GUEST_SSH}-:22 \
  -vnc $GUEST_QEMU_VNC \
  -monitor stdio

$KVMOSX_PREFIX/bin/qemu-system-x86_64 -enable-kvm -m 2048 -cpu core2duo -smp 2 -machine q35 -vga std -usb -device usb-kbd -device usb-mouse -device isa-applesmc,osk="ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc" -kernel $KVMOSX_PREFIX/lib/chameleon_svn2360_boot -smbios type=2 -device ide-drive,bus=ide.2,drive=MacHDD -drive id=MacHDD,if=none,file=$VM_PATH/$VM_NAME.img -bios $KVMOSX_PREFIX/lib/bios-mac.bin -device virtio-net,netdev=hub0port0,id=eth0 -netdev user,id=hub0port0,hostfwd=tcp:${GUEST_NATIVE_VNC}-:5900,hostfwd=tcp:${GUEST_SSH}-:22 -vnc $GUEST_QEMU_VNC -monitor stdio
```

CREATING A SNAPSHOT
===================

**WARNING:** The new snapshot file is what qemu should use! If you roll back to the
backing image then the snapshot becomes invalid / destroyed / unusable. Always
inherit snapshots from the MOST RECENT snapshot or you destroy all the previous
ones.
```
$ export SNAPSHOT_NAME="2014_05_10_post_install_xcode_clean"
$ mv $VM_PATH/$VM_NAME.img $VM_PATH/${VM_NAME}_${SNAPSHOT_NAME}.img
$ echo "Just installed. Turned off energy saving, spotlight indexing, etc. Installed XCode." > $VM_PATH/${VM_NAME}_${SNAPSHOT_NAME}.txt
$ $KVMOSX_PREFIX/bin/qemu-img create -f qcow2 -b $VM_PATH/${VM_NAME}_${SNAPSHOT_NAME}.img $VM_PATH/${VM_NAME}.img
```

GUEST SETUP
===========

1. Configure Screen Sharing (VNC) and SSH access (Settings -> Sharing -> Screen Sharing / Remote Login).
2. Install XCode from Mac App Store. Run it once to accept the license.
3. Install a JDK from Oracle (used by Jenkins -- must be a JDK since the JRE isn't headless).
4. Disable screensaver.
5. Disable Spotlight indexing of $HOME (or wherever build are happening). Settings -> Spotlight -> Privacy -> Add /Users/mixxx.
6. Make Windows shell scripts run by Jenkins do nothing: ```ln -s /bin/true /usr/local/bin/cmd```
7. In Jenkins, set prefix start slave command to: ```eval `/usr/libexec/path_helper -s` && ``` to get the PATH set correctly to include /usr/local/bin. https://issues.jenkins-ci.org/browse/JENKINS-15655
8. Create build environments (see [Mac OS X builder setup][1]):

To build an OS X 10.7 (libc++) amd64 build environment:

```
./scripts/macosx/build_environment.sh --name mixxx-2.0-x86_64-10.7 --macosx-sdk 10.11 --macosx-target 10.7 --enable-x86-64 --macosx-stdlib libc++
```

To build an OS X 10.6 (libstdc++) amd64/i386 build environment:
```
$ bash ./scripts/macosx/build_environment.sh --name intel-osx10.6 --enable-x86-64 --enable-i386 --macosx-sdk 10.11 --macosx-target 10.6 --macosx-stdlib libstdc++
```

[1] http://mixxx.org/wiki/doku.php/macosx_builder_setup
