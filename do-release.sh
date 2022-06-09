#!/bin/sh

set -eu

echo "Enter version e.g. 2.9.0"
read str
VERSION=$str
[ -n "$VERSION" ]

echo "Are you sure you want to release $VERSION? (press return)"
read str

echo "Checking for symbolic link to Windows file sharing."
[ -e fileserver ]

echo "Checking for symbolic link to ftp local directory."
[ -e ftplocal ]
  
echo "Waiting for Mac app. (press return)"
read str

echo "Checking for Mac app."
[ -d suika.app ]

echo "Building suika.exe"
cd build/mingw
#make clean
#./build-libs.sh
make
cp suika.exe ../../fileserver/
cd ../../

echo "Building suika-pro.exe"
cd build/mingw-pro
#make clean
#./build-libs.sh
make
cp suika-pro.exe ../../fileserver/
cd ../../

echo ""
echo "Waiting for code signing. (press return)"
read str
cp fileserver/suika.exe .
cp fileserver/suika-pro.exe .

echo "Building Emscripten files."
cd build/emscripten
make clean
make
cd ../../

echo "Creating Win/Mac release files."
cd build/release
make clean
make
cp suika-2.x.x-en.zip "../../ftplocal/suika-$VERSION-en.zip"
cp suika-2.x.x-jp.zip "../../ftplocal/suika-$VERSION-jp.zip"
cd ../../

echo "Creating Web version release files."
cd build/web-kit
make clean
make
cp suika2-web-kit-2.x.x-en.zip "../../ftplocal/suika2-web-kit-$VERSION-en.zip"
cp suika2-web-kit-2.x.x-jp.zip "../../ftplocal/suika2-web-kit-$VERSION-jp.zip"
cd ../../

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

echo "Release completed."
