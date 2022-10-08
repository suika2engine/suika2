#!/bin/sh

set -eu

echo "Enter version e.g. 2.9.0"
read str
VERSION=$str
[ -n "$VERSION" ]

echo ""
echo "Are you sure you want to release $VERSION? (press return)"
read str

echo ""
echo "Checking for the mountpoint of Windows file sharing."
[ -e fileserver ]

echo ""
echo "Checking for the symbolic link to ftp local directory."
[ -e ftplocal ]
  
echo ""
echo "Please prepare Mac app DMGs. (press return)"
read str

echo ""
echo "Checking for Mac apps."
[ -f fileserver/mac.dmg ]
[ -f fileserver/mac-pro.dmg ]

echo ""
echo "Building Emscripten files."
cd build/emscripten
make clean
make
cd ../../

echo ""
echo "Building suika.exe"
cd build/mingw
make clean
./build-libs.sh
make -j24
cp suika.exe ../../fileserver/
cd ../../

echo ""
echo "Building suika-pro.exe"
cd build/mingw-pro
make clean
cp -Rav ../mingw/libroot .
make -j24
cp suika-pro.exe ../../fileserver/
cd ../../

echo ""
echo "Please sign the Windows executables. (press return)"
read str
[ -f fileserver/suika.exe ]
[ -f fileserver/suika-pro.exe ]

echo ""
echo "Creating Windows/Mac release files."
cd build/release
make clean
make
cp suika-2.x.x-en.zip "../../ftplocal/suika-$VERSION-en.zip"
cp suika-2.x.x-jp.zip "../../ftplocal/suika-$VERSION-jp.zip"
cd ../../

echo ""
echo "Creating Web distribution kit release files."
cd build/web-kit
make clean
make
cp suika2-web-kit-2.x.x-en.zip "../../ftplocal/suika2-web-kit-$VERSION-en.zip"
cp suika2-web-kit-2.x.x-jp.zip "../../ftplocal/suika2-web-kit-$VERSION-jp.zip"
cd ../../

echo ""
echo "Uploading release files."
cd ftplocal
ftpupload.sh "suika-$VERSION-en.zip"
sleep 5
ftpupload.sh "suika-$VERSION-jp.zip"
sleep 5
ftpupload.sh "suika2-web-kit-$VERSION-en.zip"
sleep 5
ftpupload.sh "suika2-web-kit-$VERSION-jp.zip"
cd ..

echo ""
echo "Release completed."
