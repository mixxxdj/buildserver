
Building an Ubuntu builder
==========================

Steps to setup an Ubuntu builder:

On the machine/VM that will be the builder:
1. Install latest LTS Ubuntu server.
2. Create 'mixxx' account with standard password.

On your administration machine:
3. `sudo apt-get install git-core fabric`
4. git clone https://github.com/mixxxdj/buildserver.git --depth 1   (to save bandwidth by getting just the latest branch)
5. cd buildserver
6. fab setup_ubuntu_builder
7. Enter the SSH connection string for the builder machine when prompted
8. Wait while the environments are set up. (Can take hours.)
9. Copy release GPG key to ~/.gnupg on the builder machine
