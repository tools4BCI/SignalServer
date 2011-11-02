#!/bin/sh
dir=.deb-build
lib_dir=$dir/libtia
dev_dir=$dir/libtia-dev
ss_dir=$dir/signalserver

pckdir=packages

distribution_id=`lsb_release -i -s`
distribution_release=`lsb_release -r -s`

# create temporary build directories
mkdir -p $dir
mkdir -p $lib_dir
mkdir -p $dev_dir
mkdir -p $ss_dir

mkdir -p $lib_dir/usr/lib
mkdir -p $lib_dir/DEBIAN

mkdir -p $dev_dir/usr/lib
mkdir -p $dev_dir/usr/include/tia
mkdir -p $dev_dir/DEBIAN

mkdir -p $ss_dir/usr/bin
mkdir -p $ss_dir/usr/etc/signalserver
mkdir -p $ss_dir/usr/share/signalserver
mkdir -p $ss_dir/DEBIAN

mkdir -p $pckdir


# copy necessary files
cp -rp ./lib/libtia.so* ./$lib_dir/usr/lib/

cp ./lib/libtia.a ./$dev_dir/usr/lib/
cp -rp ./include/tia ./$dev_dir/usr/include

cp ./bin/signalserver ./$ss_dir/usr/bin/
cp ./bin/server_config.xml ./$ss_dir/usr/share/signalserver/
cp ./bin/server_config_comments.xml ./$ss_dir/usr/share/signalserver/

# current version
version=1.0

# get local architecture
architecture=`dpkg-architecture -l | grep DEB_BUILD_ARCH= | sed -e '/DEB_BUILD_ARCH=/s/DEB_BUILD_ARCH=//'`

# replace architecture, file size and version in the control file
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_tia  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$lib_dir/DEBIAN/control
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_tia_dev  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$dev_dir/DEBIAN/control
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_signalserver  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$ss_dir/DEBIAN/control


# build the SigViewer package
dpkg -b ./$lib_dir $pckdir/libtia-$version-$architecture-$distribution_id-$distribution_release.deb
dpkg -b ./$dev_dir $pckdir/libtia-dev-$version-$architecture-$distribution_id-$distribution_release.deb
dpkg -b ./$ss_dir  $pckdir/signalserver-$version-$architecture-$distribution_id-$distribution_release.deb

# delete all temporary build directories
rm -r $dir
