Host Setup
==========

As of Ubuntu 18.04, qemu provided by Ubuntu is sufficient to support macOS 10.13
High Sierra. Follow
[these instructions](https://github.com/kholia/OSX-KVM/tree/master/HighSierra)
to generate installation media and a Clover EFI bootloader.

Create a disk and boot the VM for the first time:

```
export VM_NAME=build-macosx-10.13-amd64
export VM_PATH=/opt/mixxx/vms/$VM_NAME
mkdir $VM_PATH
cd $VM_PATH
# Allocate plenty of space for the virtual disk. The qcow2 format by default starts small and grows as needed.
qemu-img create -f qcow2 $VM_PATH/$VM_NAME.img 256G

# Ports for the localhost interface on the host that forward to the guest.
export GUEST_SSH=127.0.0.1:10010
export GUEST_QEMU_VNC=127.0.0.1:10
export GUEST_NATIVE_VNC=127.0.0.1:5911

CPU_OPTIONS="+aes,+xsave,+avx,+xsaveopt,avx2,+smep"
INSTALL_MEDIA_PATH=isos/HighSierra.iso
CLOVER_PATH=isos/Clover-v2.4k-4658-x64.qcow2

qemu-system-x86_64 --enable-kvm \
  -m 4096 \
  -cpu Penryn,kvm=on,vendor=GenuineIntel,+invtsc,vmware-cpuid-freq=on,$CPU_OPTIONS \
  -machine pc-q35-2.9 \
  -smp 2,cores=1 \
  -usb -device usb-kbd -device usb-tablet \
  -device isa-applesmc,osk="ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc" \
  -drive if=pflash,format=raw,readonly,file=isos/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=isos/OVMF_VARS-1024x768.fd \
  -smbios type=2 \
  -device ich9-intel-hda -device hda-duplex \
  -device ide-drive,bus=ide.2,drive=Clover \
  -drive id=Clover,if=none,snapshot=on,format=qcow2,file=$CLOVER_PATH \
  -device ide-drive,bus=ide.1,drive=MacHDD \
  -drive id=MacHDD,if=none,file=$VM_PATH/$VM_NAME.img \
  -device ide-drive,bus=ide.0,drive=MacDVD \
  -drive id=MacDVD,if=none,snapshot=on,media=cdrom,file=$INSTALL_MEDIA_PATH \
  -netdev user,id=net0,hostfwd=tcp:${GUEST_NATIVE_VNC}-:5900,hostfwd=tcp:${GUEST_SSH}-:22 \
  -vnc $GUEST_QEMU_VNC -device e1000-82545em,netdev=net0,id=net0,mac=52:54:00:c9:18:27 \
  -monitor stdio
  ```

Follow
[these instructions](https://github.com/kholia/OSX-KVM/tree/master/HighSierra)
to install macOS (pretty self-explanatory). See "Guest Setup" below.

Creating a Snapshot
===================

**WARNING:** The new snapshot file is what qemu should use! If you roll back to the
backing image then the snapshot becomes invalid / destroyed / unusable. Always
inherit snapshots from the MOST RECENT snapshot or you destroy all the previous
ones.
```
$ export SNAPSHOT_NAME="2014_05_10_post_install_xcode_clean"
$ mv $VM_PATH/$VM_NAME.img $VM_PATH/${VM_NAME}_${SNAPSHOT_NAME}.img
$ echo "Just installed. Turned off energy saving, spotlight indexing, etc. Installed XCode." > $VM_PATH/${VM_NAME}_${SNAPSHOT_NAME}.txt
$ qemu-img create -f qcow2 -b $VM_PATH/${VM_NAME}_${SNAPSHOT_NAME}.img $VM_PATH/${VM_NAME}.img
```

Guest Setup
===========

1. Configure Screen Sharing (VNC) and SSH access (Settings -> Sharing -> Screen Sharing / Remote Login).
2. Disable Energy Saving (Settings -> Energy Saver -> Computer Sleep: Never, Display Sleep: Never, Uncheck "Put hard disks to sleep whenever possible")
3. Install XCode from [developer.apple.com](https://developer.apple.com/xcode/downloads/). Run it once to accept the license.
4. Run `xcode-select --install` from a Terminal to install the commandline tools.
5. Install a JDK from Oracle (used by Jenkins -- must be a JDK since the JRE isn't headless).
6. Disable screensaver.
7. Disable Spotlight indexing of $HOME (or wherever builds are happening). Settings -> Spotlight -> Privacy -> Add $HOME.
8. Make Windows shell scripts run by Jenkins do nothing: ```ln -s /usr/bin/true /usr/local/bin/cmd```
9. In Jenkins, set prefix start slave command to: ```eval `/usr/libexec/path_helper -s` && ``` to get the PATH set correctly to include /usr/local/bin. https://issues.jenkins-ci.org/browse/JENKINS-15655
10. Add an SSH public key to `~/.ssh/authorized_keys` so that Jenkins can log in. Add the private key to Jenkins.
11. Create a `~/Library/Keychains/Mixxx.keychain` keychain using Keychain Access.

To test everything is working, create a build environment manually. To build an
OS X 10.11 (libc++) amd64 build environment:

```
$ git clone https://github.com/mixxxdj/buildserver
$ cd buildserver
$ ./scripts/macosx/build_environment.sh --name mixxx-2.2-x86_64-10.11 --macosx-sdk 10.13 --macosx-target 10.11 --enable-x86-64 --macosx-stdlib libc++
```
