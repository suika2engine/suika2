Suika2 Pro for Linux (Qt6)
==========================
This is Suika2 Pro for Linux.

It runs on Linux as well as Windows and macOS.
However, while we prefer static linking on Windows,
it doesn't seem to work because Qt6 plugins are not statically linkable.
So the author decided not to use this port on Windows.

* Preparation:
  * Ubuntu
    * Install Qt6 (or Qt Creator)
    ```
    apt-get install qt6-base-dev qt6-multimedia-dev
    ```
  * Windows/macOS (for testing)
    * Install Qt Creator and setup Qt 6.2.4

* Build with command line
  * Run the following commands in this directory:
  ```
  ./make-deps.sh
  mkdir build
  cd build
  cmake ..
  make
  cp ..
  cp build/suika-pro ../../suika-pro
  ```

* Build with Qt Creator
  * Run `./make-deps.sh` in this directory
  * Open Qt Creator
  * Open `build/suika-pro-qt/CMakeLists.txt` from Qt Creator
  * Build

* [For Debian] Build with command line (shared version)
  * Run the following commands in this directory:
  ```
  ./make-deps-shared.sh
  mkdir build
  cd build
  cmake ..
  make
  cp ..
  cp build/suika-pro ../../suika-pro
  ```
