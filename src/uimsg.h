/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * uimsg.c: UI message management.
 */

#ifndef SUIKA_UIMSG_H
#define SUIKA_UIMSG_H

enum {
	UIMSG_INFO,
	UIMSG_WARN,
	UIMSG_ERROR,
	UIMSG_CANNOT_OPEN_LOG,
	UIMSG_EXIT,
	UIMSG_TITLE,
	UIMSG_DELETE,
	UIMSG_OVERWRITE,
	UIMSG_DEFAULT,
#ifdef WIN
	UIMSG_WIN_NO_DIRECT3D,
	UIMSG_WIN_NO_OPENGL,
	UIMSG_WIN_SMALL_DISPLAY,
	UIMSG_WIN_MENU_FILE,
	UIMSG_WIN_MENU_VIEW,
	UIMSG_WIN_MENU_QUIT,
	UIMSG_WIN_MENU_FULLSCREEN,
#endif
};

const wchar_t *get_ui_message(int id);

#endif
