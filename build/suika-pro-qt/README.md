Suika2 Pro (Qt6 re-implementation)
==================================
This is a re-implementation of Suika2 Pro.
It runs on Windows, macOS and Linux.

* Preparation:
  * Windows
    * Install Qt Creator and setup Qt 6.2.4
  * macOS
    * Install Qt Creator and setup Qt 6.2.4
  * Linux
    * Install Qt6 (or Qt Creator)
    ```
    apt-get install qt6-base-dev qt6-multimedia-dev
    ```

* Build with Qt Creator
  * Run `./make-deps.sh` in this directory
  * Open Qt Creator
  * Open `build/suika-pro-qt/CMakeLists.txt` from Qt Creator
  * Build

* Build with command line
  * To build with command line, run the following commands in this directory:
  ```
  ./make-deps.sh
  mkdir build
  cd build
  cmake ..
  make
  cp ..
  cp build/suika-pro ../../suika-pro
  ```
