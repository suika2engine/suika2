# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             suika

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/cmd_bg.c
             src/main/cpp/cmd_bgm.c
             src/main/cpp/cmd_ch.c
             src/main/cpp/cmd_click.c
             src/main/cpp/cmd_gosub.c
             src/main/cpp/cmd_goto.c
             src/main/cpp/cmd_if.c
             src/main/cpp/cmd_load.c
             src/main/cpp/cmd_menu.c
             src/main/cpp/cmd_message.c
             src/main/cpp/cmd_retrospect.c
             src/main/cpp/cmd_return.c
             src/main/cpp/cmd_se.c
             src/main/cpp/cmd_select.c
             src/main/cpp/cmd_set.c
             src/main/cpp/cmd_switch.c
             src/main/cpp/cmd_vol.c
             src/main/cpp/cmd_wait.c
             src/main/cpp/conf.c
             src/main/cpp/event.c
             src/main/cpp/history.c
             src/main/cpp/log.c
             src/main/cpp/main.c
             src/main/cpp/mixer.c
             src/main/cpp/ndkfile.c
             src/main/cpp/ndkglyph.c
             src/main/cpp/ndkimage.c
             src/main/cpp/ndkmain.c
             src/main/cpp/ndkwave.c
             src/main/cpp/save.c
             src/main/cpp/script.c
             src/main/cpp/stage.c
             src/main/cpp/vars.c )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       suika
                       jnigraphics

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )