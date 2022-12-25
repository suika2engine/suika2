/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

#include "types.h"

/*
 * Printer. (for print() intrinsic)
 */
int wms_printf(const char *s, ...)
{
	UNUSED_PARAMETER(s);
	return 0;
}

/*
 * Reader. (for readline() intrinsic)
 */
int wms_readline(char *buf, size_t len)
{
	UNUSED_PARAMETER(buf);
	UNUSED_PARAMETER(len);
	return 0;
}
