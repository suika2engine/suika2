Suika2 on Visual Studio 2022
============================
You can build Suika2 using Visual Studio 2022.

Note that this is currently for debugging purpose only.
MSVC doesn't generate SSE3/SSE4.1/SSE4.2 code but we would like to utilize them.

## Building on Windows
* Prerequisites
  * Before building Suika2, you have to build the libraries first
  * Open `Developer Command Prompt for Visual Studio 2022`
  * Navigate to `build/vs` and type the following command:
  ```
  build-libs.bat
  ```
* Open `build/vs/suika.sln`
  * Build

## Building on WSL2
* In the terminal, navigate to `build/vs` and just type the following command:
```
make
```

## The following information is a memorandum:
* zlib
  * `cd build/vs`
  * `tar xzf ../libsrc/zlib-1.2.11.tar.gz`
  * `mv zlib-1.2.11 zlib`
* libpng
  * `cd build/vs`
  * `tar xzf ../libsrc/libpng-1.6.35.tar.gz`
  * `mv libpng-1.6.35 libpng`
  * Open `libpng/projects/vstudio/zlib.props`
    * Change `ZLibSrcDir` to `..\..\..\..\zlib`
    * Save and close Visual Studio
  * Open `libpng/projects/vstudio/vstudio.sln`
    * Select `Release Library` and `Win32` target
    * zlib -> Property
      * C/C++
        * General
          * Warning Level -> /W0
          * Treat Warning As Error -> /WX-
        * All Options
          * Disable Specified Warnings -> $(DisableSpecificWarnings);4127;4131;4242;4244;4061;5045
    * libpng -> Property
      * C/C++
        * General
          * Warning Level -> /W0
          * Treat Warning As Error -> /WX-
        * All Options
          * Disable Specified Warnings -> $(DisableSpecificWarnings);4127;4131;4242;4244;4061;5045
    * Delete following projects in the solution:
      * pngtest
      * pngvalid
      * pngstest
      * pngunknown
  * Build
* libjpeg
  * `cd build/vs`
  * `tar xzf ../libsrc/jpegsrc.v9e.tar.gz`
  * `mv jpeg-9e jpeg`
  * `cd jpeg`
  * `nmake /f makefile.vs setupcopy-v16`
  * Open `jpeg.sln`
  * Select `Release` and `Win32` target
  * Build solution
* libogg
  * `cd build/vs`
  * `tar xzf ../libsrc/libogg-1.3.3.tar.gz`
  * `mv libogg-1.3.3 libogg`
  * Open `libogg/win32/VS2015/libogg_static.sln`
    * Select `Release` and `Win32` target
    * Build
* libvorbis
  * `cd build/vs`
  * `tar xzf ../libsrc/libvorbis-1.3.6.tar.gz`
  * `mv libvorbis-1.3.6 libvorbis`
  * Open `libvorbis/win32/VS2010/vorbis_static.sln`
    * Select `Release` and `Win32` target
    * Build
* freetype
  * `cd build/vs`
  * `tar xzf ../libsrc/freetype-2.9.1.tar.gz`
  * `mv freetype-2.9.1 freetype`
  * Open `freetype/builds/windows/vc2010/freetype.sln`
    * Select `Release Static` and `Win32` target
    * Build
