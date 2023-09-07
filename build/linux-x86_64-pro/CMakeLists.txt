cmake_minimum_required(VERSION 3.5)

project(suika2-pro-qt VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Gui)

set(PROJECT_SOURCES
	main.cpp
	mainwindow.cpp
	mainwindow.h
	openglwidget.cpp
	openglwidget.h
        glwrapper.cpp
        mainwindow.ui
)

add_subdirectory(deps)
include_directories(deps .)

qt_add_executable(suika2-pro-qt
	MANUAL_FINALIZATION
	${PROJECT_SOURCES}
)

target_link_libraries(
	suika2-pro-qt
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
)

if(WIN32)
  find_library(DSOUND_LIB dsound)
  if(NOT DSOUND_LIB)
    add_compile_definitions(NO_SOUND)
    message(STATUS "Warning: use dummy sound output.")
  else()
    target_link_libraries(suika2-pro-qt PRIVATE dsound)
  endif()
endif()
if(UNIX AND NOT APPLE)
  find_library(ASOUND_LIB asound)
  if(NOT ASOUND_LIB)
    message(FATAL_ERROR "asound library not found")
  endif()
  target_link_libraries(suika2-pro-qt PRIVATE asound)
endif()

include(GNUInstallDirs)
install(TARGETS suika2-pro-qt
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(suika2-pro-qt)
