#!/bin/sh
dir=.deb-build
ss_dir=$dir/signalserver

pckdir=packages

distribution_id=`lsb_release -i -s`
distribution_release=`lsb_release -r -s`

# create temporary build directories
mkdir -p $dir
mkdir -p $ss_dir
mkdir -p $ss_dir/usr/bin
mkdir -p $ss_dir/usr/etc/signalserver
mkdir -p $ss_dir/usr/share/signalserver
mkdir -p $ss_dir/DEBIAN

mkdir -p $pckdir


# copy necessary files
cp ./bin/signalserver ./$ss_dir/usr/bin/
cp ./bin/server_config.xml ./$ss_dir/usr/share/signalserver/
cp ./bin/server_config_comments.xml ./$ss_dir/usr/share/signalserver/

# current version
version=1.0

# get local architecture
architecture=`dpkg-architecture -l | grep DEB_BUILD_ARCH= | sed -e '/DEB_BUILD_ARCH=/s/DEB_BUILD_ARCH=//'`

# replace architecture, file size and version in the control file
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_signalserver  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$ss_dir/DEBIAN/control

# build the SigViewer package
dpkg -b ./$ss_dir  $pckdir/signalserver-$version-$architecture-$distribution_id-$distribution_release.deb

# delete all temporary build directories
rm -r $dir
