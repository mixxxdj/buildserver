import os

from fabric.api import *

LOGIN_KEY_NAME = 'build-login'
LOGIN_KEY_PATH = os.path.join('keys', LOGIN_KEY_NAME)
UBUNTU_VERSION = '18.04.1'
UBUNTU_NAME = 'bionic'
MACOSX_QEMU_ROOT = '/usr/local/osx'
MACOSX_INSTALL_ISO = 'ElCapitan.iso'
# https://wiki.ubuntu.com/Releases for current list
PBUILDER_DISTS = ['bionic']
PBUILDER_ARCHS = ['amd64', 'i386']

# Because our SConscripts are a little sloppy, the host system has to have
# all the dependencies installed even though we aren't building the package
# on the host system (it's built in the pbuilder chroots).
# TODO(rryan): Remove duplication with pbuilderrc.
# TODO(rryan): Fix this mess.
MIXXX_DEBIAN_DEPENDENCIES = [
    'debhelper',
    'libasound2-dev',
    'libchromaprint-dev',
    'libfaad-dev',
    'libhidapi-dev',
    'libid3tag0-dev',
    'libjack-dev',
    'libmad0-dev',
    'libmp4v2-dev',
    'libogg-dev',
    'libopus-dev',
    'libopusfile-dev',
    'libportmidi-dev',
    'libprotobuf-dev',
    'libqt4-dev',
    'libqt4-opengl-dev',
    'libqt4-sql-sqlite',
    'libqt5opengl5-dev',
    'libqt5svg5-dev',
    'libqt5xmlpatterns5-dev',
    'librubberband-dev',
    'libshout-dev',
    'libsndfile1-dev',
    'libsoundtouch-dev',
    'libsqlite3-dev',
    'libtag1-dev',
    'libupower-glib-dev',
    'libusb-1.0-0-dev',
    'libvorbis-dev',
    'portaudio19-dev',
    'protobuf-compiler',
    'qt5-default',
    'qtdeclarative5-dev',
    'qtdeclarative5-dev-tools',
    'qtscript5-dev',
    'scons',
    'vamp-plugin-sdk',
]

env.user = 'mixxx'
env.key_filename = LOGIN_KEY_PATH

def setup_host():
    sudo('apt-get -y install qemu-kvm')
    local('mkdir -p vms isos keys')
    make_ssh_login_key()

def make_ssh_login_key(key_filename=LOGIN_KEY_PATH):
    if not os.path.exists(key_filename):
        local('ssh-keygen -t rsa -b 4096 -f %s -C %s' % (key_filename, LOGIN_KEY_NAME))
    else:
        print('Login key already exists.')

def make_ubuntu_autoinstaller():
    if not os.path.exists(os.path.join(
            'isos', 'ubuntu-%s-server-amd64-auto.iso' % UBUNTU_VERSION)):
        local('./scripts/linux-ubuntu/make-ubuntu-autoinstaller.sh '
              '--mixxx_ssh_key %s '
              '--ubuntu_name %s '
              '--ubuntu version %s' %
              ('%s.pub' % LOGIN_KEY_PATH, UBUNTU_NAME, UBUNTU_VERSION))

def _builder_common(name):
    vm_path = os.path.join('vms', name)
    if os.path.exists(vm_path):
        raise Exception('Builder \'%s\' already exists!' % name)
    local('mkdir ' + vm_path)
    vm_disk = '{}.img'.format(name)

    disk_space = prompt("How much disk (XXG)?")
    with lcd(vm_path):
        local('qemu-img create -f qcow2 {vm_disk} {disk_space}'.format(
            vm_disk=vm_disk, disk_space=disk_space))

    ram = prompt('How much RAM in MB (XXXX)?')
    cores = prompt('How many cores?')
    guest_ssh = prompt('Guest SSH host:port (127.0.0.1:10010)?')
    guest_vnc = prompt('Guest VNC host:display (127.0.0.1:14)?')

    return vm_path, vm_disk, disk_space, ram, cores, guest_ssh, guest_vnc

def make_ubuntu_builder(name):
    make_ubuntu_autoinstaller()
    vm_path, vm_disk, disk_space, ram, cores, guest_ssh, guest_vnc = (
        _builder_common(name))

    install_media = os.path.join(
        'isos', 'ubuntu-%s-server-amd64-auto.iso' % UBUNTU_VERSION)

    with lcd(vm_path):
        local('echo "#!/usr/bin/env bash" > run.sh')
        local('echo "qemu-system-x86_64 --enable-kvm -m {ram} -cpu core2duo -smp {cores} -device ide-drive,drive=LinHDD -drive id=LinHDD,if=none,file={vm_path}/{vm_disk} -device virtio-net,netdev=hub0port0,id=eth0 -netdev user,id=hub0port0,hostfwd=tcp:{guest_ssh}-:22 -vnc {guest_vnc} -monitor stdio -boot c -cdrom {install_media}" >> run.sh'.format(ram=ram, cores=cores, vm_path=vm_path, vm_disk=vm_disk, guest_ssh=guest_ssh, guest_vnc=guest_vnc, install_media=install_media))
        local('chmod +x run.sh')

def make_macosx_builder(name):
    vm_path, vm_disk, disk_space, ram, cores, guest_ssh, guest_vnc = (
        _builder_common(name))
    guest_native_vnc = prompt('Guest native VNC host:display?')
    install_media = os.path.join('isos', MACOSX_INSTALL_ISO)

    with lcd(vm_path):
        local('echo "#!/usr/bin/env bash" > run.sh')
        local('echo "export PATH={qemu_root}/bin:\\$PATH" >> run.sh'.format(qemu_root=MACOSX_QEMU_ROOT))
        local('echo "sudo su -- -c \\\"echo 1 > /sys/module/kvm/parameters/ignore_msrs\\\"" >> run.sh')
        local('echo "qemu-system-x86_64 --enable-kvm -m {ram} -cpu core2duo,vendor=GenuineIntel -machine q35 -vga std -usb -device usb-kbd -device usb-mouse -device isa-applesmc,osk=\\\"ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc\\\" -kernel isos/enoch_rev2795_boot -smbios type=2 -device ide-drive,bus=ide.2,drive=MacHDD -drive id=MacHDD,if=none,file={vm_path}/{vm_disk} -netdev user,id=hub0port0,hostfwd=tcp:{guest_native_vnc}-:5900,hostfwd=tcp:{guest_ssh}-:22 -vnc {guest_vnc} -device e1000-82545em,netdev=hub0port0,id=mac_vnet0 -monitor stdio -smp {cores} -boot c -device ide-drive,bus=ide.0,drive=MacDVD -drive id=MacDVD,if=none,snapshot=on,file={install_media}" >> run.sh'.format(ram=ram, cores=cores, vm_path=vm_path, vm_disk=vm_disk, guest_ssh=guest_ssh, guest_vnc=guest_vnc, guest_native_vnc=guest_native_vnc, install_media=install_media))
        local('chmod +x run.sh')

# During setup:
# "KernelBooter_kexts"="Yes" "CsrActiveConfig"="103"
# Format with disk utility
# Settings -> Sharing -> SSH/VNC
# XCode, homebrew?

# Create file /Extra/org.chameleon.boot.plist:
"""
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
          "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Timeout</key>
  <string>5</string>
</dict>
"""

def run_builder(name):
    vm_path = os.path.join('vms', name)
    local(os.path.join(vm_path, 'run.sh'))

def install_fake_cmd():
    # Jenkins needs to "run" Windows shell jobs. Make /bin/true point to
    # /usr/local/bin/cmd.
    sudo("ln -s /bin/true /usr/local/bin/cmd")

def setup_ubuntu_builder():
    install_fake_cmd()
    install_packages()
    install_mixxx_dependencies()
    setup_pbuilder()

def install_mixxx_dependencies():
    sudo('apt-get -y install ' + ' '.join(MIXXX_DEBIAN_DEPENDENCIES))

def install_packages():
    # default-jre-headless: for running Jenkins node
    # xvfb: for running tests
    sudo('apt-get -y install build-essential git-core default-jre-headless xvfb')

def update_pbuilderrc():
    put('scripts/linux-ubuntu/pbuilderrc')
    run('mv pbuilderrc .pbuilderrc')

def setup_pbuilder():
    sudo('apt-get -y install pbuilder debootstrap devscripts')

def create_pbuilder_chroots():
    update_pbuilderrc()
    for dist in PBUILDER_DISTS:
        for arch in PBUILDER_ARCHS:
            sudo('DIST=%s ARCH=%s pbuilder create' % (dist, arch))

def update_pbuilder_chroots():
    update_pbuilderrc()
    for dist in PBUILDER_DISTS:
        for arch in PBUILDER_ARCHS:
            sudo('DIST=%s ARCH=%s pbuilder update' % (dist, arch))
