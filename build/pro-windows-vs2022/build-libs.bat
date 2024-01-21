@echo off

echo Removing the library directories...
rd /s /q zlib > NUL 2>&1
rd /s /q libpng > NUL 2>&1
rd /s /q jpeg > NUL 2>&1
rd /s /q libogg > NUL 2>&1
rd /s /q libvorbis > NUL 2>&1
rd /s /q freetype > NUL 2>&1
rd /s /q bzip2 > NUL 2>&1
rd /s /q libwebp > NUL 2>&1

echo Extracting zlib...
tar -xzf ../libsrc/zlib-1.2.11.tar.gz
ren zlib-1.2.11 zlib

echo Building libpng...
tar -xzf ../libsrc/libpng-1.6.35.tar.gz
ren libpng-1.6.35 libpng
copy patch\libpng\projects\vstudio\libpng\libpng.vcxproj libpng\projects\vstudio\libpng\libpng.vcxproj
copy patch\libpng\projects\vstudio\pnglibconf\pnglibconf.vcxproj libpng\projects\vstudio\pnglibconf\pnglibconf.vcxproj
copy patch\libpng\projects\vstudio\zlib\zlib.vcxproj libpng\projects\vstudio\zlib\zlib.vcxproj
copy patch\libpng\projects\vstudio\vstudio.sln libpng\projects\vstudio\vstudio.sln
copy patch\libpng\projects\vstudio\zlib.props libpng\projects\vstudio\zlib.props
msbuild libpng\projects\vstudio\vstudio.sln /t:build /p:Configuration="Release Library";Platform="Win32"

echo Building jpeg...
tar -xzf ../libsrc/jpegsrc.v9e.tar.gz
ren jpeg-9e jpeg
cd jpeg
nmake /f makefile.vs setupcopy-v16
cd ..
copy patch\jpeg\jpeg.sln jpeg\jpeg.sln
copy patch\jpeg\jpeg.vcxproj jpeg\jpeg.vcxproj
msbuild jpeg\jpeg.sln /t:build /p:Configuration="Release";Platform="Win32"

echo Building bzip2...
tar -xzf ../libsrc/bzip2-1.0.6.tar.gz
ren bzip2-1.0.6 bzip2
cd bzip2
nmake -f makefile.msc
cd ..

echo Building libwebp...
tar -xzf ../libsrc/libwebp-1.3.2.tar.gz
ren libwebp-1.3.2 libwebp
cd libwebp
nmake /f makefile.vc CFG=release-static ARCH=x86 RTLIBCFG=dll
cd ..

echo Building libogg...
tar -xzf ../libsrc/libogg-1.3.3.tar.gz
ren libogg-1.3.3 libogg
copy patch\libogg\libogg_static.sln libogg\win32\VS2015\libogg_static.sln
copy patch\libogg\libogg_static.vcxproj libogg\win32\VS2015\libogg_static.vcxproj
msbuild libogg\win32\VS2015\libogg_static.sln /t:build /p:Configuration="Release";Platform="Win32"

echo Building libvorbis...
tar -xzf ../libsrc/libvorbis-1.3.6.tar.gz
ren libvorbis-1.3.6 libvorbis
copy patch\libvorbis\vorbis_static.sln libvorbis\win32\VS2010\vorbis_static.sln
copy patch\libvorbis\libvorbis_static.vcxproj libvorbis\win32\VS2010\libvorbis\libvorbis_static.vcxproj
copy patch\libvorbis\libvorbisfile_static.vcxproj libvorbis\win32\VS2010\libvorbisfile\libvorbisfile_static.vcxproj
msbuild libvorbis\win32\VS2010\vorbis_static.sln /t:build /p:Configuration="Release";Platform="Win32"

echo Building freetype...
tar -xzf ../libsrc/freetype-2.13.2.tar.gz
ren freetype-2.13.2 freetype
copy patch\freetype\freetype.sln freetype\builds\windows\vc2010\freetype.sln
copy patch\freetype\freetype.vcxproj freetype\builds\windows\vc2010\freetype.vcxproj
msbuild freetype\builds\windows\vc2010\freetype.sln /t:build /p:Configuration="Release Static";Platform="Win32"
