cmake_minimum_required(VERSION 3.5)

project(suika-pro VERSION 0.1 LANGUAGES CXX C)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")

# We use .ui file translation.
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# We use C++17 because Qt6 supports it.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# We use Qt6.
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Gui Multimedia)

# We use the following preprocessor macros.
add_compile_definitions(
  USE_DEBUGGER
  USE_QT
  NO_SSE_VERSIONING
)
if(CMAKE_TOOLCHAIN_FILE)
  # Add -DMINGW_CROSS if we are cross compiling for mingw.
  # TODO: not required for macOS.
  add_compile_definitions(
    MINGW_CROSS
  )
endif()

# The following are Pro source files that implements a debugger HAL for Qt6.
set(PROJECT_SOURCES
  main.cpp
  mainwindow.cpp
  mainwindow.h
  openglwidget.cpp
  openglwidget.h
  glwrapper.cpp
  mainwindow.ui
)

# We use additional CL flags for MSVC.
if(MSVC)
  # Use utf-8 source files.
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

  # Suppress warnings for string.h function calls.
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Add Suika2 main engine. (recursively include the dependency libraries)
add_subdirectory(deps)
include_directories(deps .)

# Add linker options if we are cross compiling for mingw.
if(CMAKE_TOOLCHAIN_FILE)
  add_link_options(
    -static-libgcc
    -static-libstdc++
  )
endif()

# The following is our output executable.
qt_add_executable(
  suika-pro
  MANUAL_FINALIZATION
  ${PROJECT_SOURCES}
)
if(NOT CMAKE_TOOLCHAIN_FILE)
  # Dynamic linking for a self compilation on Linux:
  target_link_libraries(
    suika-pro
    PRIVATE
    suika2
    zlib
    png
    jpeg
    vorbisfile
    vorbis
    ogg
    freetype
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Multimedia
  )
else()
  # Static linking for a cross compilation for Windows and macOS:
  target_link_libraries(
    suika-pro
    PRIVATE
    suika2
    zlib
    png
    # jpeg is redundant: libqjpeg will be statically linked.
    vorbisfile
    vorbis
    ogg
    freetype
    dxgi
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Multimedia
  )
endif()

# TODO: probably we don't need the following for now.
include(GNUInstallDirs)
install(TARGETS suika-pro
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finish.
qt_finalize_executable(suika-pro)
