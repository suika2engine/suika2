/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-05-27 Created.
 *  - 2023-12-08 Refactored.
 */

#ifndef SUIKA_SUIKA_H
#define SUIKA_SUIKA_H

 /* Compiler-specific features */
#include "types.h"

/* Standard C (ANSI-C/C89/C90) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <assert.h>

/* HAL (Hardware Abstraction Layer) */
#include "hal.h"

/* Subsystems */
#include "anime.h"	/* The animation subsystem */
#include "conf.h"	/* The configuration subsystem */
#include "event.h"	/* The event handling subsystem */
#include "file.h"	/* The file subsystem */
#include "glyph.h"	/* The glyph rendering and text layout subsystem */
#include "gui.h"	/* The GUI subsystem */
#include "history.h"	/* The history subsystem */
#include "image.h"	/* The image subsystem */
#include "log.h"	/* The log subsystem */
#include "main.h"	/* The main subsystem */
#include "mixer.h"	/* The mixer subsystem */
#include "motion.h"	/* The motion subsystem */
#include "save.h"	/* The save subsystem */
#include "scbuf.h"	/* The scan conversion buffer subsystem */
#include "script.h"	/* The scenario script subsystem */
#include "seen.h"	/* The seen flag subsystem */
#include "stage.h"	/* The stage rendering subsystem */
#include "uimsg.h"	/* The UI message subsystem */
#include "vars.h"	/* The variable subsystem */
#include "wave.h"	/* The sound stream subsystem */
#include "wms.h"	/* The WMS subsystem */

/* Suika2 Pro */
#if defined(USE_EDITOR)
#include "pro.h"
#endif

/*
 * Directory Names
 */

/* The name of the config file directory */
#define CONF_DIR	"conf"

/* The name of the background image directory */
#define BG_DIR		"bg"

/* The name of the character image directory */
#define CH_DIR		"ch"

/* The name of the background music directory */
#define BGM_DIR		"bgm"

/* The name of the sound effect directory */
#define SE_DIR		"se"

/* The name of the voice directory */
#define CV_DIR		"cv"

/* The name of the system image directory */
#define CG_DIR		"cg"

/* The name of the font file firectory */
#define FONT_DIR	"font"

/* The name of the scenario file firectory */
#define SCRIPT_DIR	"txt"

/* The name of the GUI file firectory */
#define GUI_DIR		"gui"

/* The name of the rule image directory */
#define RULE_DIR	"rule"

/* The name of the anime file firectory */
#define ANIME_DIR	"anime"

/* The name of the video file firectory */
#define MOV_DIR		"mov"

/* The name of the WMS file firectory */
#define WMS_DIR		"wms"

/* The name of the save file firectory */
#define SAVE_DIR	"sav"

/*
 * File Names
 */

/* The log file name. */
#define LOG_FILE	"log.txt"

/* The package file name. */
#define PACKAGE_FILE	"data01.arc"

/* The config file name */
#define CONFIG_FILE	"config.txt"

/* The initial scenario file. */
#define INIT_FILE	"init.txt"

#endif
