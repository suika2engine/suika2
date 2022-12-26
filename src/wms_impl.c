/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

#include "suika.h"
#include "wms.h"

/*
 * FFI function declaration
 */

bool s2_get_variable(struct wms_runtime *rt);
bool s2_set_variable(struct wms_runtime *rt);

/*
 * FFI function table 
 */

struct wms_ffi_func_tbl ffi_func_tbl[] = {
	{s2_get_variable, "s2_get_variable", {"index", NULL}},
	{s2_set_variable, "s2_set_variable", {"index", "value", NULL}},
};

#define FFI_FUNC_TBL_SIZE (sizeof(ffi_func_tbl) / sizeof(ffi_func_tbl[0]))

/*
 * Forward declaration
 */

/* TODO: Declare static functions here. */

/*
 * FFI function definition
 */

/*
 * Get a value of a specified variable.
 */
bool s2_get_variable(struct wms_runtime *rt)
{
	struct wms_value *index;
	int index_i;
	int value;

	assert(rt != NULL);

	/* Get the argument pointer. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;

	/* Get the argument value. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;

	/* Get the value of the Suika2 variable. */
	value = get_variable(index_i);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", value, NULL))
		return false;

	return true;
}

bool s2_set_variable(struct wms_runtime *rt)
{
	struct wms_value *index, *value;
	int index_i, value_i;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;
	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument values. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;
	if (!wms_get_int_value(rt, value, &value_i))
		return false;

	/* Set the value of the Suika2 variable. */
	set_variable(index_i, value_i);

	return true;
}

/*
 * FFI function registration
 */

bool register_s2_functions(struct wms_runtime *rt)
{
	if (!wms_register_ffi_func_tbl(rt, ffi_func_tbl, FFI_FUNC_TBL_SIZE)) {
		log_wms_runtime_error("", 0, wms_get_runtime_error_message(rt));
		return false;
	}
	return true;
}

/*
 * For intrinsic
 */

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
