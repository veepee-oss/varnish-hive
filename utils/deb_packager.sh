#!/bin/sh

VERSION=$(grep 'VERSION=' configure.ac | sed -e 's/VERSION=//')
DIR=$(pwd)

echo $DIR
echo "Build .deb package - VERSION: $VERSION"

#echo "Set debian/changelog version according to configure.ac..."
#sed -i -e "s/^varnish-hive (0.5)/varnish-hive (${VERSION})/" debian/changelog
#echo "Done."

echo "Get Varnish sources and prepare them for compilation..."
bash ./autogen.sh
bash ./configure
echo "Done."

echo "Create varnish-hive_${VERSION}.orig.tar.gz..."
cd ..
tar -zcvf varnish-hive_${VERSION}.orig.tar.gz varnish-hive/ > /dev/null
echo "Done."

echo "Start .deb build process..."
cd varnish-hive/
debuild -us -uc
echo "Done."

echo "Done."
