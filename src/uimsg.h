/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * uimsg.c: Internationalized UI message subsystem
 */

#ifndef SUIKA_UIMSG_H
#define SUIKA_UIMSG_H

enum {
	UIMSG_YES,
	UIMSG_NO,
	UIMSG_INFO,
	UIMSG_WARN,
	UIMSG_ERROR,
	UIMSG_CANNOT_OPEN_LOG,
	UIMSG_EXIT,
	UIMSG_TITLE,
	UIMSG_DELETE,
	UIMSG_OVERWRITE,
	UIMSG_DEFAULT,
	UIMSG_NO_SOUND_DEVICE,
	UIMSG_NO_GAME_FILES,
#ifdef SUIKA_TARGET_WIN32
	UIMSG_WIN32_NO_DIRECT3D,
	UIMSG_WIN32_SMALL_DISPLAY,
	UIMSG_WIN32_MENU_FILE,
	UIMSG_WIN32_MENU_VIEW,
	UIMSG_WIN32_MENU_QUIT,
	UIMSG_WIN32_MENU_FULLSCREEN,
#endif
};

const char *get_ui_message(int id);

#endif
