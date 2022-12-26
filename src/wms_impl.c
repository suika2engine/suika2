/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

#include "suika.h"
#include "wms.h"

#include <stdio.h>

/*
 * Printer. (for print() intrinsic)
 */
int wms_printf(const char *s, ...)
{
	char buf[1024];
	va_list ap;
	int ret;

	va_start(ap, s);
	ret = vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	if (strcmp(buf, "\n") == 0)
		return 0;

	log_warn(buf);
	return ret;
}
