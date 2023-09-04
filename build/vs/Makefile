MSBUILD=`wslpath -w '/mnt/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/msbuild.exe'`

all:
	if [ ! -e zlib ] || [ ! -e libpng ] || [ ! -e libogg ] || [ ! -e libvorbis ] || [ ! -e jpeg ] || [ ! -e freetype ]; then ./build-libs.sh; fi
	powershell.exe -Command "& \"$(MSBUILD)\" suika.sln /t:build /p:Configuration=\"Release\" /p:Platform=\"x86\""
