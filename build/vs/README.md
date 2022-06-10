BUILD
=====

You can build Suika2 using Visual Studio 2022.

Note that this is currently for debug purpose only because Visual Studio 2022
doesn't generate SSE3/SSE4.1/SSE4.2 code.

If you use Suika2 in Japanese mode, make sure you run Visual Studio 2022
in Japanese environment in order to process Japanese strings.

Prior to build Suika2, you have to build libraries.

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
