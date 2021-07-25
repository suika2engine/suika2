Emscripten
==========

You can deploy your game as a Web application using Emscripten.

* Prerequisites
    * Use UNIX-like environment (MSYS2/Linux/macOS)
        * You need `make` and `python`
    * Install Emscripten (`emcc` command)
    * Generate `data01.arc` file using package-win.exe
        * Steps are described in `doc/tutorial.md`

* Instructions
    * `cd build/emscripten`
    * `make`
    * Copy your `data01.arc` to `build/emscripten/html/`
    * `make run`
    * Visit `http://localhost:8000/html/` by a browser
