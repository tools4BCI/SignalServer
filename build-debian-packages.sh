#!/bin/sh
dir=.deb-build
lib_dir=$dir/libtia
dev_dir=$dir/libtia-dev
ss_dir=$dir/signalserver

pckdir=packages

# create temporary build directories
mkdir -p $lib_dir/usr/lib
mkdir -p $lib_dir/DEBIAN

mkdir -p $dev_dir/usr/lib
mkdir -p $dev_dir/usr/include/tia
mkdir -p $dev_dir/DEBIAN

mkdir -p $ss_dir/usr/bin
mkdir -p $ss_dir/usr/local/etc/signalserver
mkdir -p $ss_dir/DEBIAN

mkdir $pckdir


# copy necessary files
cp ./lib/libtia.so* ./$lib_dir/usr/lib/
cp ./lib/libtiaclient.so* ./$lib_dir/usr/lib/

cp ./lib/libtia.a ./$dev_dir/usr/lib/
cp ./lib/libtiaclient.a ./$dev_dir/usr/lib/
cp -r ./include/config ./$dev_dir/usr/include/tia
cp -r ./include/definitions ./$dev_dir/usr/include/tia
cp -r ./include/datapacket ./$dev_dir/usr/include/tia
cp -r ./include/network ./$dev_dir/usr/include/tia
cp -r ./include/signalserver ./$dev_dir/usr/include/tia
cp -r ./include/signalserver-client ./$dev_dir/usr/include/tia

cp ./bin/signalserver ./$ss_dir/usr/bin/
cp ./bin/server_config.xml ./$ss_dir/usr/local/etc/signalserver/

# current version
version=0.1

# get local architecture
architecture=`dpkg-architecture -l | grep DEB_BUILD_ARCH= | sed -e '/DEB_BUILD_ARCH=/s/DEB_BUILD_ARCH=//'`

# replace architecture, file size and version in the control file
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_tia  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$lib_dir/DEBIAN/control
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_tia_dev  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$dev_dir/DEBIAN/control
sed -e '/Architecture: /s/<architecture-via-script>/'$architecture'/' ./misc/deb_control_template_signalserver  | sed -e '/Version: /s/<version>/'$version'/'  |  sed -e '/Replaces: /s/<version>/'$version'/'  > ./$ss_dir/DEBIAN/control


# build the SigViewer package
dpkg -b ./$lib_dir $pckdir/libtia-$version-$architecture.deb
dpkg -b ./$dev_dir $pckdir/libtia-$version-$architecture-dev.deb
dpkg -b ./$ss_dir  $pckdir/signalserver-$version-$architecture.deb

# delete all temporary build directories
rm -r $dir
