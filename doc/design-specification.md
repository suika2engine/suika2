***

# Software Design Specification for Suika2

Author: ktabata

Modified By:

Organization: the Suika2 Development Team

Updated: 31th October 2022

Created: 29th October 2022

Revision: 0.2

***

# Introduction

This document outlines the software design specification (SDS) for Suika2 in English.
It is intended to help programmers understand Suika2's high-level design and get started quickly.

***

# Components

Suika2 consists of the following components:

* Hardware Abstraction Layer (HAL)
* Platform Independent Part (PIP)

```
+---------------+
| cmd_*.c (PIP) |
+---------------+
| main.c (PIP)  |
+---------------+
| event.c (PIP) |
+---------------+
| HAL           |
+---------------+
| OS API        |
+---------------+
```

# HAL

A HAL is a component implemented on a per-platform basis.

## Supported Platforms

Currently, the project has six HALs.

* Windows
* macOS
* Web (Emscripten)
* iOS
* Android (NDK)
* Linux

## HAL Modules

The HALs consist of the following modules:

* Windows
    * `winmain.c` ... Entry Point 
    * `d3drender.cc` ... Direct3D Renderer
    * `glrender.c` ... OpenGL Renderer
    * `dsound.c` ... DirectSound Player
    * `dsvideo.cc` ... Video Player
* macOS
    * `nsmain.m` ... Entry Point
    * `glrender.c` ... OpenGL Renderer
    * `aunit.c` ... AudioUnit Player
* Web (Emscripten)
    * `emmain.c` ... Entry Point
    * `glrender.c` ... OpenGL Renderer
    * `emopenal.c` ... OpenAL Player
* iOS
    * `iosmain.m` ... Entry Point
    * `glrender.c` ... OpenGL Renderer
    * `emopenal.c` ... OpenAL Player
* Android
    * `ndkmain.c` ... Entry Point
    * `ndkfile.c` ... File Operation
    * `ndkwave.c` ... Sound Playback
    * `glrender.c` ... OpenGL Renderer
* Linux
    * `x11main.c` ... Entry Point
    * `glrender.c` ... OpenGL Renderer
    * `asound.c` ... ALSA Player

## HAL Functionality

A HAL must provide the following functionalities:

* Initialization
* Main Loop
* Event Handling
* Logging
* Path Manipulation
* GPU Operations
* Time Measurement
* Confirmation Dialogs
* Sound Playback
* Video Playback
* Window Manipulation
* System Locale

# PIP

The PIP provides the following functionalities:

* Main Flow
    * Event Handlers (`event.c`)
    * Command Dispatcher (`main.c`)
    * Command Implementation (`cmd_*.c`)
* Common Process for Most Platforms
    * File and Package (`file.c`)
* Common Process for All Platforms
    * Rendering (`stage.c`)
    * Logging (`log.c`)
    * Config Parser (`config.c`)
    * Script Parser (`script.c`)
    * GUI Parser and Execution (`gui.c`)

# Application Flow

A HAL runs the main loop.
In the main loop, HAL dispatches the events to the functions implemented in `event.c`.
The following functions are the event handling functions that are implemented in `event.c`

```
bool on_event_init(void);
void on_event_cleanup(void);
bool on_event_frame(int *x, int *y, int *w, int *h);
void on_event_key_press(int key);
void on_event_key_release(int key);
void on_event_mouse_press(int button, int x, int y);
void on_event_mouse_release(int button, int x, int y);
void on_event_mouse_move(int x, int y);
void on_event_mouse_scroll(int n);
```

`on_event_frame()` function is called every frame.
In this function, the program calls `game_loop_iter()` function that is implemented in `main.c`.

`game_loop_iter()` calls `dispatch_command()`.
In `dispatch_command()`, each command implementation such as `message_command()` is called.

# Command Implementation Function

The command implementation functions are implemented in `cmd_*.c` files.

A command implementation function is called every frames while the command is executed.
A function, `is_in_command_repetition()`,
is provided to distinguish between the first frame and the frames that will be executed repeatedly after that.

When the command quits, the command implementation function just calls `move_to_next_command()`.