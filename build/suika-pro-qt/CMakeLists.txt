cmake_minimum_required(VERSION 3.5)

project(suika-pro VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Qt6.
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Gui Multimedia)

# Preprocessor macros.
add_compile_definitions(
        USE_DEBUGGER
        USE_QT
        NO_SSE_VERSIONING
)

# Pro source files.
set(PROJECT_SOURCES
	main.cpp
	mainwindow.cpp
	mainwindow.h
	openglwidget.cpp
	openglwidget.h
        glwrapper.cpp
        mainwindow.ui
)

# For Visual Studio
if(MSVC)
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Add Suika2 main engine and dependency libraries.
add_subdirectory(deps)
include_directories(deps .)

# Main executable.
qt_add_executable(suika-pro
	MANUAL_FINALIZATION
	${PROJECT_SOURCES}
)
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

# Maybe we don't need this:
include(GNUInstallDirs)
install(TARGETS suika-pro
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(suika-pro)
