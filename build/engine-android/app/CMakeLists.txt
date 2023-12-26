cmake_minimum_required(VERSION 3.4.1)
project(suika-lib)

# Find Android NDK log library.
find_library(log-lib log)

# We make libsuika.so
add_library(
  suika
  SHARED

  # HAL
  src/main/cpp/ndkmain.c
  src/main/cpp/ndkfile.c
  src/main/cpp/glrender.c
  src/main/cpp/slsound.c

  # platform-independent
  src/main/cpp/anime.c
  src/main/cpp/conf.c
  src/main/cpp/event.c
  src/main/cpp/glyph.c
  src/main/cpp/gui.c
  src/main/cpp/history.c
  src/main/cpp/image.c
  src/main/cpp/log.c
  src/main/cpp/main.c
  src/main/cpp/mixer.c
  src/main/cpp/readimage.c
  src/main/cpp/readpng.c
  src/main/cpp/readjpeg.c
  src/main/cpp/readwebp.c
  src/main/cpp/save.c
  src/main/cpp/scbuf.c
  src/main/cpp/script.c
  src/main/cpp/seen.c
  src/main/cpp/stage.c
  src/main/cpp/vars.c
  src/main/cpp/wave.c
  src/main/cpp/wms_core.c
  src/main/cpp/wms_impl.c
  src/main/cpp/wms_lexer.yy.c
  src/main/cpp/wms_parser.tab.c
  src/main/cpp/cmd_anime.c
  src/main/cpp/cmd_bg.c
  src/main/cpp/cmd_bgm.c
  src/main/cpp/cmd_ch.c
  src/main/cpp/cmd_cha.c
  src/main/cpp/cmd_chapter.c
  src/main/cpp/cmd_chs.c
  src/main/cpp/cmd_click.c
  src/main/cpp/cmd_gosub.c
  src/main/cpp/cmd_goto.c
  src/main/cpp/cmd_gui.c
  src/main/cpp/cmd_if.c
  src/main/cpp/cmd_layer.c
  src/main/cpp/cmd_load.c
  src/main/cpp/cmd_message.c
  src/main/cpp/cmd_pencil.c
  src/main/cpp/cmd_return.c
  src/main/cpp/cmd_se.c
  src/main/cpp/cmd_set.c
  src/main/cpp/cmd_setconfig.c
  src/main/cpp/cmd_setsave.c
  src/main/cpp/cmd_shake.c
  src/main/cpp/cmd_skip.c
  src/main/cpp/cmd_switch.c
  src/main/cpp/cmd_video.c
  src/main/cpp/cmd_vol.c
  src/main/cpp/cmd_wait.c
  src/main/cpp/cmd_wms.c
)
target_link_libraries(
  suika
  GLESv3
  png
  jpeg
  webp
  freetype
  bz2
  z
  ogg
  vorbis
  OpenSLES
  ${log-lib}
)

# Add bzip2
add_subdirectory(src/main/cpp/bzip2)
include_directories(src/main/cpp/bzip2)

# Add libpng
option(HAVE_LD_VERSION_SCRIPT OFF)
set(HAVE_LD_VERSION_SCRIPT OFF)
add_compile_definitions(SKIP_INSTALL_ALL=YES)
add_subdirectory(src/main/cpp/libpng)
include_directories(src/main/cpp/libpng)

# Add jpeg
add_subdirectory(src/main/cpp/jpeg)
include_directories(src/main/cpp/jpeg)

# Add libwebp
add_subdirectory(src/main/cpp/libwebp)
include_directories(src/main/cpp/libwebp)

# Add freetype2
add_subdirectory(src/main/cpp/freetype)
include_directories(src/main/cpp/freetype/include)

# Add libogg
add_subdirectory(src/main/cpp/libogg)
include_directories(src/main/cpp/libogg/include)

# Add libvorbis
add_subdirectory(src/main/cpp/libvorbis)
include_directories(src/main/cpp/libvorbis/include)
