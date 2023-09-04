# Path to msbuild.exe
MSBUILD='/mnt/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/msbuild.exe'
MSBUILD=`wslpath -w "$MSBUILD"`

# Search nmake.exe
NMAKE=`find "/mnt/c/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC" -name 'nmake.exe' | grep 'bin/Hostx86/x86'`
NMAKE=`wslpath -w "$NMAKE"`

# Create a working directory in user home on Windows
WORKDIR=/mnt/c/Users/`powershell.exe '$env:UserName' | tr -d '\r\n'`/suika2-vs-tmp
rm -rf "$WORKDIR"
mkdir "$WORKDIR"

cp ../libsrc/zlib-1.2.11.tar.gz "$WORKDIR/"
cp ../libsrc/libpng-1.6.35.tar.gz "$WORKDIR/"
cp ../libsrc/jpegsrc.v9e.tar.gz "$WORKDIR/"
cp ../libsrc/libogg-1.3.3.tar.gz "$WORKDIR/"
cp ../libsrc/libvorbis-1.3.6.tar.gz "$WORKDIR/"
cp ../libsrc/freetype-2.9.1.tar.gz "$WORKDIR/"
cp -R patch  "$WORKDIR/"

SAVED_DIR=`pwd`
cd "$WORKDIR"

echo Extracting zlib...
tar xzf zlib-1.2.11.tar.gz
mv zlib-1.2.11 zlib

echo Building libpng...
tar xzf libpng-1.6.35.tar.gz
mv libpng-1.6.35 libpng
cp patch/libpng/projects/vstudio/libpng/libpng.vcxproj libpng/projects/vstudio/libpng/libpng.vcxproj
cp patch/libpng/projects/vstudio/pnglibconf/pnglibconf.vcxproj libpng/projects/vstudio/pnglibconf/pnglibconf.vcxproj
cp patch/libpng/projects/vstudio/zlib/zlib.vcxproj libpng/projects/vstudio/zlib/zlib.vcxproj
cp patch/libpng/projects/vstudio/vstudio.sln libpng/projects/vstudio/vstudio.sln
cp patch/libpng/projects/vstudio/zlib.props libpng/projects/vstudio/zlib.props
powershell.exe -Command "& '$MSBUILD' libpng/projects/vstudio/vstudio.sln /t:build '/p:Configuration=\"Release Library\";Platform=\"Win32\"'"

echo Building jpeg...
tar xzf jpegsrc.v9e.tar.gz
mv jpeg-9e jpeg
cd jpeg
powershell.exe -Command "& '$NMAKE' /f makefile.vs setupcopy-v16"
cd ..
cp patch/jpeg/jpeg.sln jpeg/jpeg.sln
cp patch/jpeg/jpeg.vcxproj jpeg/jpeg.vcxproj
powershell.exe -Command "& '$MSBUILD' jpeg/jpeg.sln /t:build '/p:Configuration=\"Release\";Platform=\"Win32\"'"

echo Building libogg...
tar xzf libogg-1.3.3.tar.gz
mv libogg-1.3.3 libogg
cp patch/libogg/libogg_static.sln libogg/win32/VS2015/libogg_static.sln
cp patch/libogg/libogg_static.vcxproj libogg/win32/VS2015/libogg_static.vcxproj
powershell.exe -Command "& '$MSBUILD' libogg/win32/VS2015/libogg_static.sln /t:build '/p:Configuration=\"Release\";Platform=\"Win32\"'"

echo Building libvorbis...
tar xzf libvorbis-1.3.6.tar.gz
mv libvorbis-1.3.6 libvorbis
cp patch/libvorbis/vorbis_static.sln libvorbis/win32/VS2010/vorbis_static.sln
cp patch/libvorbis/libvorbis_static.vcxproj libvorbis/win32/VS2010/libvorbis/libvorbis_static.vcxproj
cp patch/libvorbis/libvorbisfile_static.vcxproj libvorbis/win32/VS2010/libvorbisfile/libvorbisfile_static.vcxproj
powershell.exe -Command "& '$MSBUILD' libvorbis/win32/VS2010/vorbis_static.sln /t:build '/p:Configuration=\"Release\";Platform=\"Win32\"'"

echo Building freetype...
tar xzf freetype-2.9.1.tar.gz
mv freetype-2.9.1 freetype
cp patch/freetype/freetype.sln freetype/builds/windows/vc2010/freetype.sln
cp patch/freetype/freetype.vcxproj freetype/builds/windows/vc2010/freetype.vcxproj
powershell.exe -Command "& '$MSBUILD' freetype/builds/windows/vc2010/freetype.sln /t:build '/p:Configuration=\"Release Static\";Platform=\"Win32\"'"

cd "$SAVED_DIR"
cp -R "$WORKDIR/zlib" .
cp -R "$WORKDIR/libpng" .
cp -R "$WORKDIR/jpeg" .
cp -R "$WORKDIR/libogg" .
cp -R "$WORKDIR/libvorbis" .
cp -R "$WORKDIR/freetype" .
