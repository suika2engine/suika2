/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 */

#ifndef SUIKA_SUIKA_H
#define SUIKA_SUIKA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "conf.h"
#include "event.h"
#include "file.h"
#include "glyph.h"
#include "history.h"
#include "image.h"
#include "log.h"
#include "main.h"
#include "mixer.h"
#include "platform.h"
#include "scbuf.h"
#include "script.h"
#include "save.h"
#include "stage.h"
#include "vars.h"
#include "wave.h"

/*
 * 固定のディレクトリ名
 */

/* 背景イメージのディレクトリ */
#define BG_DIR		"bg"

/* キャライメージのディレクトリ */
#define CH_DIR		"ch"

/* BGMのディレクトリ */
#define BGM_DIR		"bgm"

/* SEのディレクトリ */
#define SE_DIR		"se"

/* ボイスのディレクトリ */
#define CV_DIR		"cv"

/* その他のCGのディレクトリ */
#define CG_DIR		"cg"

/* スクリプトのディレクトリ */
#define SCRIPT_DIR	"txt"

/* フォントのディレクトリ */
#define FONT_DIR	"font"

/* 設定ファイルのディレクトリ */
#define CONF_DIR	"conf"

/* セーブデータのディレクトリ */
#define SAVE_DIR	"sav"

/*
 * 固定のファイル名
 */

/* ログファイル名 */
#define LOG_FILE	"log.txt"

/* パッケージファイル名 */
#define PACKAGE_FILE	"data01.arc"

/* 設定ファイル名 */
#define PROP_FILE	"config.txt"

/* 初期スクリプト */
#define INIT_FILE	"init.txt"

#endif
