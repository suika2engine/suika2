***

# Software Design Specification for Suika2

Author: ktabata

Modified By:

Organization: the Suika2 Development Team

Updated: 30th October 2022

Created: 29th October 2022

Revision: 0.1

***

# Introduction

This document outlines the software design specification (SDS) for Suika2 in English.
It is intended to help programmers understand Suika2's high-level design and get started quickly.

***

# Components

Suika2 consists of the following components:

* Hardware Abstraction Layer (HAL)
* Platform Independent Part (PIP)

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
    * File and Package
* Common Process for All Platforms
    * Logging
    * Script Parser
    * Config Parser
    * GUI Parser and Execution
