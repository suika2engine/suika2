BUILD(3D)
=========

You can build Suika2 Direct3D version using Visual Studio 2022.

Currently, 2D drawing is enough fast and we don't have a reason to
move to 3D version immediately.

If you use Suika2 in Japanese mode, make sure you run Visual Studio 2022
in Japanese environment in order to process Japanese strings.

Prior to build Suika2, you have to build libraries.

* zlib
    * `cd build/msvc-3d`
    * `tar xzf ../libsrc/zlib-1.2.11.tar.gz`
    * `mv zlib-1.2.11 zlib`

* libpng
    * `cd build/msvc-3d`
    * `tar xzf ../libsrc/libpng-1.6.35.tar.gz`
    * `mv libpng-1.6.35 libpng`
    * Open `libpng/projects/vstudio/zlib.props`
        * Change `ZLibSrcDir` to `..\..\..\..\zlib`
        * Save and close Visual Studio
    * Open `libpng/projects/vstudio/vstudio.sln`
        * Select `Release Library` target
        * zlib -> Build -> Property -> C/C++ -> Warning Level -> /W0
        * libpng -> Build -> Property -> C/C++ -> Warning Level -> /W0
        * Delete following projects in the solution:
            * pngtest
            * pngvalid
            * pngstest
            * pngunknown
        * Build

* libogg
    * `cd build/msvc-3d`
    * `tar xzf ../libsrc/libogg-1.3.3.tar.gz`
    * `mv libogg-1.3.3 libogg`
    * Open `libogg/win32/VS2015/libogg_static.sln`
        * Select `Release` target
        * Build

* libvorbis
    * `cd build/msvc-3d`
    * `tar xzf ../libsrc/libvorbis-1.3.6.tar.gz`
    * `mv libvorbis-1.3.6 libvorbis`
    * Open `libvorbis/win32/VS2010/vorbis_static.sln`
        * Select `Release` target
        * Build

* freetype
    * `cd build/msvc-3d`
    * `tar xzf ../libsrc/freetype-2.9.1.tar.gz`
    * `mv freetype-2.9.1 freetype`
    * Open `freetype/builds/windows/vc2010/freetype.sln`
        * Select `Release Static` target
        * Build
