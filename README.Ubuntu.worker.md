
Building an Ubuntu builder
==========================

Steps to setup an Ubuntu builder:

1. Install latest LTS Ubuntu server.
2. Create 'mixxx' account with standard password.
3. `sudo apt-get install git-core`
4. git clone https://github.com/mixxxdj/buildserver.git
5. ./buildserver/scripts/linux-ubuntu/setup-builder.sh
6. Copy release GPG key to ~/.gnupg
