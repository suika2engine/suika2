/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

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

#include "anime.h"
#include "conf.h"
#include "event.h"
#include "file.h"
#include "glyph.h"
#include "gui.h"
#include "history.h"
#include "image.h"
#include "log.h"
#include "main.h"
#include "mixer.h"
#include "platform.h"
#include "save.h"
#include "scbuf.h"
#include "script.h"
#include "seen.h"
#include "stage.h"
#include "vars.h"
#include "wave.h"

/*
 * 固定のディレクトリ名
 */

/* 背景イメージのディレクトリ */
#define BG_DIR		"bg"

/* ルールイメージのディレクトリ */
#define RULE_DIR	"rule"

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

/* GUIファイルのディレクトリ */
#define GUI_DIR		"gui"

/* セーブデータのディレクトリ */
#define SAVE_DIR	"sav"

/* 動画データのディレクトリ */
#define MOV_DIR		"mov"

/* WMSのディレクトリ */
#define WMS_DIR		"wms"

/* アニメのディレクトリ */
#define ANIME_DIR	"anime"

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

/*
 * キャプチャ/リプレイ
 */
#if defined(USE_CAPTURE) || defined(USE_REPLAY)
#define CSV_HEADER	"time,PNG,X,Y,left,right,lclick,rclick,return," \
			"space,escape,up,down,pageup,pagedown,control\n"
#endif

#endif
