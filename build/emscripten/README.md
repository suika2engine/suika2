Emscripten
==========

You can deploy your game as a Web application using Emscripten.

* Prerequisites
    * Use UNIX-like environment (MSYS2/Linux/macOS)
        * You need `make` and `python`
    * Install Emscripten (`emcc` command)
    * Generate `data01.arc` file using package-win.exe

* Steps for local Web server
    * `cd suika2/build/emscripten`
    * `make`
    * Copy your `data01.arc` to `suika2/build/emscripten/html/`
    * `make run`
    * Browse `http://localhost:8000/html/`

* Additional step for real Web server
    * Edit absolute path in `suika2/build/emscripten/pre.js`
