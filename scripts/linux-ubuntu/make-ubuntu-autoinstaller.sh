#! /bin/bash

# Thanks to Michael Utech.
# http://web.archive.org/web/20140316120853/http://www.utech.de/2013/05/shell-script-creating-a-cd-for-unattended-ubuntu-server-installations

MIXXX_ROOT=""
MIXXX_SSH_KEY_PATH=""

#
# Ubuntu release selection
#
release_name="bionic"
release_version="18.04.1"
# Do not use a "live" server ISO.
release_variant="server"
release_architecture="amd64"

usage() { echo "Usage: $0 [--mixxx_ssh_key <key>] [--mixxx_root <root>] [--ubuntu_version <$release_version>] [[--ubuntu_name <$release_name>]" 1>&2; exit 1; }

while getopts ":-:" o; do
    case "${o}" in
        -)
            case "${OPTARG}" in
                mixxx_ssh_key)
                    MIXXX_SSH_KEY_PATH="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                mixxx_root)
                    MIXXX_ROOT="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                ubuntu_version)
                    release_version="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                ubuntu_name)
                    release_name="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
            esac;;
        *)
            echo "Usage: ${o}"
            ;;
    esac
done
shift $((OPTIND-1))

if [[ -z "$MIXXX_ROOT" ]]; then
    MIXXX_ROOT="/opt/mixxx"
fi

if [[ -z "$MIXXX_SSH_KEY_PATH" ]]; then
    MIXXX_SSH_KEY_PATH="$MIXXX_ROOT/keys/builder-login.pub"
fi

if [ ! -d $MIXXX_ROOT ]; then
   echo "Mixxx root does not exist: $MIXXX_ROOT"
   exit 1
fi

if [ ! -f $MIXXX_SSH_KEY_PATH ]; then
   echo "Login SSH key does not exist: $MIXXX_SSH_KEY_PATH"
   exit 1
fi

MIXXX_SSH_KEY=$(cat $MIXXX_SSH_KEY_PATH)

#
# General configuration
#
basedir="$MIXXX_ROOT/isos"
downloads="$MIXXX_ROOT/isos"
tmpdir="${TMPDIR:-/tmp}"
builddir="$tmpdir/build.$$"
mntdir="$tmpdir/mnt.$$"


release_base_url="http://cdimage.ubuntu.com/releases/"
release_base_name="ubuntu-$release_version-$release_variant-$release_architecture"
release_image_file="$release_base_name.iso"
release_url="$release_base_url/$release_version/release/$release_image_file"

#
# Target settings
#
target_base_name="${release_base_name}-auto"
target_directory="$basedir"
target_image_file="$target_base_name.iso"

progress() {
    echo "$*" >&2
}

error() {
    code="$1"; shift
    echo "ERROR: $*" >&2
    exit $code
}

create_directory() {
    path="$1"
    if [ ! -d "$path" ]; then
	progress "Creating directory $path..."
	mkdir -p "$path" || error 2 "Failed to create directory $path"
    fi
}

extract_iso() {
    archive="$1"
    if [ ! -r "$archive" ]; then
	error 1 "Cannot read ISO image $archive."
    fi
    directory="$2"
    if [ ! -d "$directory" ]; then
	mkdir "$directory" || exit 2 "Cannot extract CD to $directory"
    fi

    progress "Mounting image $archive (you may be asked for your password to authorize)..."
    create_directory "$mntdir"
    sudo mount -r -o loop "$archive" "$mntdir" || error 2 "Failed to mount image $archive"

    progress "Copying image contents..."
    cp -rT "$mntdir" "$directory" || error 2 "Failed to copy content of image $archive to $directory"
    chmod -R u+w "$directory"

    progress "Unmounting image $archive from $mntdir..."
    sudo umount "$mntdir"
    rmdir "$mntdir"
}

preset_language() {
    progress "Presetting language to 'en'..."
    echo "en" >"isolinux/lang" || error 2 "Failed to write $(pwd)/isolinux/lang"
}

create_kscfg() {
    if [ ! -f "ks.cfg" ]; then
	progress "Create ks.cfg file..."
	cat >"ks.cfg" <<EOF
# System language
lang en_US

# Language modules to install
langsupport en_US

# System keyboard
keyboard gb_mac_intl

# System mouse
mouse

# System timezone
timezone --utc America/Los_Angeles

# Root password
rootpw --disabled

# Initial user
user mixxx --fullname "Mixxx Buildbot" --password mixxxbuild

# Reboot after installation
reboot

# Use text mode install
text

# Install OS instead of upgrade
install

# Use CDROM installation media
cdrom

# System bootloader configuration
bootloader --location=mbr

# Clear the Master Boot Record
zerombr yes

# Partition clearing information
clearpart --all --initlabel

# Disk partitioning information
part / --fstype ext4 --size 1 --grow
part swap --recommended

# System authorization infomation
auth  --useshadow  --enablemd5

# Network information
network --bootproto=dhcp --device=eth0

# Firewall configuration
firewall --disabled

# Do not configure the X Window System
skipx

%packages

@ ubuntu-server
openssh-server

%post

# Disable password-based login over SSH.
cat >> /etc/ssh/sshd_config << EOF_sshd_config
PasswordAuthentication no
EOF_sshd_config

# Install the provided SSH key for mixxx.
mkdir -p /home/mixxx/.ssh
cat >> /home/mixxx/.ssh/authorized_keys << EOF_authorized_keys
$MIXXX_SSH_KEY
EOF_authorized_keys
chown -R mixxx:mixxx /home/mixxx/.ssh

# Enable passwordless sudo for mixxx.
echo "mixxx ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/mixxx

%end
EOF
    fi
}

create_kspreseed() {
    if [ ! -f "ks.preseed" ]; then
	progress "Create ks.preseed file..."
	cat >"ks.preseed" <<EOF
d-i partman/confirm_write_new_label boolean true
d-i partman/choose_partition \
select Finish partitioning and write changes to disk
d-i partman/confirm boolean true
EOF
    fi
}

patch_txtcfg() {
    (cd "isolinux";
	patch -p0 <<EOF
--- txt.cfg.orig   2016-05-28 14:38:11.261556255 -0700
+++ txt.cfg    2016-05-29 11:46:35.054984997 -0700
@@ -2,7 +2,7 @@
 label install
   menu label ^Install Ubuntu Server
   kernel /install/vmlinuz
-  append  file=/cdrom/preseed/ubuntu-server.seed vga=788 initrd=/install/initrd.gz quiet ---
+  append  file=/cdrom/preseed/ubuntu-server.seed initrd=/install/initrd.gz ks=cdrom:/ks.cfg preseed/file=/cdrom/ks.preseed ---
 label maas
   menu label ^Install MAAS Region Controller
   kernel /install/vmlinuz
EOF
    )
}

patch_isolinuxcfg() {
    (cd "isolinux";
	patch -p0 <<EOF
*** isolinux.cfg.orig	2013-05-14 10:20:37.000000000 +0200
--- isolinux.cfg	2013-05-14 10:20:50.000000000 +0200
***************
*** 2,6 ****
  include menu.cfg
  default vesamenu.c32
  prompt 0
! timeout 300
  ui gfxboot bootlogo
--- 2,6 ----
  include menu.cfg
  default vesamenu.c32
  prompt 0
! timeout 1
  ui gfxboot bootlogo
EOF
    )
}

modify_release() {
    preset_language && \
	create_kscfg && \
	create_kspreseed && \
	patch_txtcfg && \
	patch_isolinuxcfg
}

create_image() {
    if [ ! -f "$target_directory/$target_image_file" ]; then
	if [ ! -f "$downloads/$release_image_file" ]; then
	    progress "Downloading Ubuntu $release_name $release_variant..."
	    curl "$release_url" -o "$downloads/$release_image_file"
	fi
	create_directory "$builddir"
	extract_iso "$downloads/$release_image_file" "$builddir"
	(cd "$builddir" && modify_release
	) || error 2 "Failed to modify image"

	create_directory "$target_directory"
	progress "Creating ISO image $target_image_file..."
	mkisofs -D -r -V "UNATTENDED_UBUNTU" -cache-inodes -J -l \
	    -b isolinux/isolinux.bin \
	    -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 \
	    -boot-info-table \
	    -o "$target_directory/$target_image_file" \
	    "$builddir" || error 2 "Failed to create image $target_image_file"
	if [ "x$builddir" != x -a "x$builddir" != "x/" ]; then
	    rm -rf "$builddir"
	fi
    else
        error 1 "$target_directory/$target_image_file already exists."
    fi
}

create_image
