/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

#include "suika.h"
#include "wms.h"

/*
 * Stage save area
 *  - FIXME: memory leaks when the app successfully exits.
 */

static char *saved_layer_file_name[LAYER_EFFECT4 + 1];
static int saved_layer_x[LAYER_EFFECT4 + 1];
static int saved_layer_y[LAYER_EFFECT4 + 1];
static int saved_layer_alpha[LAYER_EFFECT4 + 1];

/*
 * FFI function declaration
 */

static bool s2_get_variable(struct wms_runtime *rt);
static bool s2_set_variable(struct wms_runtime *rt);
static bool s2_get_name_variable(struct wms_runtime *rt);
static bool s2_set_name_variable(struct wms_runtime *rt);
static bool s2_random(struct wms_runtime *rt);
static bool s2_set_config(struct wms_runtime *rt);
static bool s2_reflect_msgbox_and_namebox_config(struct wms_runtime *rt);
static bool s2_reflect_font_config(struct wms_runtime *rt);
static bool s2_clear_history(struct wms_runtime *rt);
static bool s2_clear_msgbox(struct wms_runtime *rt);
static bool s2_clear_namebox(struct wms_runtime *rt);
static bool s2_hide_msgbox(struct wms_runtime *rt);
static bool s2_hide_namebox(struct wms_runtime *rt);
static bool s2_save_global(struct wms_runtime *rt);
static bool s2_push_stage(struct wms_runtime *rt);
static bool s2_pop_stage(struct wms_runtime *rt);

/*
 * FFI function table 
 */

struct wms_ffi_func_tbl ffi_func_tbl[] = {
	{s2_get_variable, "s2_get_variable", {"index", NULL}},
	{s2_set_variable, "s2_set_variable", {"index", "value", NULL}},
	{s2_get_name_variable, "s2_get_name_variable", {"index", NULL}},
	{s2_set_name_variable, "s2_set_name_variable", {"index", "value", NULL}},
	{s2_random, "s2_random", {NULL}},
	{s2_set_config, "s2_set_config", {"key", "value", NULL}},
	{s2_reflect_msgbox_and_namebox_config, "s2_reflect_msgbox_and_namebox_config", {NULL}},
	{s2_reflect_font_config, "s2_reflect_font_config", {NULL}},
	{s2_clear_history, "s2_clear_history", {NULL}},
	{s2_clear_msgbox, "s2_clear_msgbox", {NULL}},
	{s2_clear_namebox, "s2_clear_namebox", {NULL}},
	{s2_hide_msgbox, "s2_hide_msgbox", {NULL}},
	{s2_hide_namebox, "s2_hide_namebox", {NULL}},
	{s2_save_global, "s2_save_global", {NULL}},
	{s2_push_stage, "s2_push_stage", {NULL}},
	{s2_pop_stage, "s2_pop_stage", {NULL}},
};

#define FFI_FUNC_TBL_SIZE (sizeof(ffi_func_tbl) / sizeof(ffi_func_tbl[0]))

/*
 * Forward declaration
 */

/* TODO: Declare static functions here. */

/*
 * FFI function definition
 */

/* Get the value of the specified variable. */
static bool s2_get_variable(struct wms_runtime *rt)
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

/* Set the value of the specified variable. */
static bool s2_set_variable(struct wms_runtime *rt)
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

/* Get the value of the specified name variable. */
static bool s2_get_name_variable(struct wms_runtime *rt)
{
	struct wms_value *index;
	const char *value;
	int index_i;

	assert(rt != NULL);

	/* Get the argument pointer. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;

	/* Get the argument value. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;

	/* Get the value of the Suika2 name variable. */
	value = get_name_variable(index_i);

	/* Set the return value. */
	if (!wms_make_str_var(rt, "__return", value, NULL))
		return false;

	return true;
}

/* Set the value of the specified name variable. */
static bool s2_set_name_variable(struct wms_runtime *rt)
{
	struct wms_value *index, *value;
	const char *value_s;
	int index_i;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "index", &index))
		return false;
	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument values. */
	if (!wms_get_int_value(rt, index, &index_i))
		return false;
	if (!wms_get_str_value(rt, value, &value_s))
		return false;

	/* Set the value of the Suika2 name variable. */
	if (!set_name_variable(index_i, value_s))
		return false;

	return true;
}

/* Returns a random number that ranges from 0 to 99999. */
static bool s2_random(struct wms_runtime *rt)
{
	int rand_value;

	assert(rt != NULL);

	rand_value = (int)((float)rand() / (float)RAND_MAX * 99999);

	/* Set the return value. */
	if (!wms_make_int_var(rt, "__return", rand_value, NULL))
		return false;

	return true;
}

/* Set the value of the config. */
static bool s2_set_config(struct wms_runtime *rt)
{
	struct wms_value *key, *value;
	const char *key_s, *value_s;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "key", &key))
		return false;
	if (!wms_get_var_value(rt, "value", &value))
		return false;

	/* Get the argument values. */
	if (!wms_get_str_value(rt, key, &key_s))
		return false;
	if (!wms_get_str_value(rt, value, &value_s))
		return false;

	/* Set the value of the Suika2 variable. */
	if (!overwrite_config(key_s, value_s))
		return false;

	return true;
}

/* Reflect the changed configs for the message box and the name box. */
static bool s2_reflect_msgbox_and_namebox_config(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Update. */
	if (!update_msgbox_and_namebox())
		return false;

	return true;
}

/* Reflect the changed font configs. */
static bool s2_reflect_font_config(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/*
	 * This function does nothing and exists for the compatibility.
	 * Now we change font in overwrite_config_font_file().
	 */

	return true;
}

/* Clear the message history. */
static bool s2_clear_history(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Clear the message history. */
	clear_history();

	return true;
}

/* Clear the message box. */
static bool s2_clear_msgbox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Clear the message box. */
	fill_msgbox();

	return true;
}

/* Clear the name box. */
static bool s2_clear_namebox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Clear the name box. */
	fill_namebox();

	return true;
}

/* Hide the message box. */
static bool s2_hide_msgbox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Hide the message box. */
	show_msgbox(false);

	return true;
}

/* Hide the name box. */
static bool s2_hide_namebox(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	/* Hide the name box. */
	show_namebox(false);

	return true;
}

/* Save the global data. */
static bool s2_save_global(struct wms_runtime *rt)
{
	UNUSED_PARAMETER(rt);

	save_global_data();

	return true;
}

/* Push the stage. */
static bool s2_push_stage(struct wms_runtime *rt)
{
	const char *s;
	int i;

	UNUSED_PARAMETER(rt);

	for (i = LAYER_BG; i <= LAYER_EFFECT4; i++) {
		/* Exclude the following layers. */
		switch (i) {
		case LAYER_MSG:		/* fall-thru */
		case LAYER_NAME:	/* fall-thru */
		case LAYER_CLICK:	/* fall-thru */
		case LAYER_AUTO:	/* fall-thru */
		case LAYER_SKIP:
			continue;
		default:
			break;
		}

		saved_layer_x[i] = get_layer_x(i);
		saved_layer_y[i] = get_layer_y(i);
		saved_layer_alpha[i] = get_layer_alpha(i);

		if (saved_layer_file_name[i] != NULL) {
			free(saved_layer_file_name[i]);
			saved_layer_file_name[i] = NULL;
		}

		s = get_layer_file_name(i);
		if (s != NULL) {
			saved_layer_file_name[i] = strdup(s);
			if (saved_layer_file_name[i] == NULL) {
				log_memory();
				return false;
			}
		} else {
			saved_layer_file_name[i] = NULL;
		}
	}

	return true;
}

/* Pop the stage. */
static bool s2_pop_stage(struct wms_runtime *rt)
{
	struct image *img;
	int i;

	UNUSED_PARAMETER(rt);

	for (i = LAYER_BG; i <= LAYER_EFFECT4; i++) {
		/* Exclude the following layers. */
		switch (i) {
		case LAYER_MSG:		/* fall-thru */
		case LAYER_NAME:	/* fall-thru */
		case LAYER_CLICK:	/* fall-thru */
		case LAYER_AUTO:	/* fall-thru */
		case LAYER_SKIP:
			continue;
		default:
			break;
		}

		if (i == LAYER_BG && saved_layer_file_name[i] == NULL) {
			/* Restore an empty background. */
			img = create_initial_bg();
			if (img == NULL)
				return false;
			set_layer_file_name(i, NULL);
			set_layer_image(i, img);
		} else if (i == LAYER_BG &&
			   saved_layer_file_name[i][0] == '#') {
			/* Restore a color background. */
			img = create_image_from_color_string(
				conf_window_width,
				conf_window_height,
				&saved_layer_file_name[i][1]);
			if (img == NULL)
				return false;
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		} else if (i == LAYER_BG) {
			/* Restore an image background. */
			if (strncmp(saved_layer_file_name[i], "cg/", 3) == 0) {
				img = create_image_from_file(
					CG_DIR,
					&saved_layer_file_name[i][3]);
			} else {
				img = create_image_from_file(
					BG_DIR,
					saved_layer_file_name[i]);
			}
			if (img == NULL)
				return false;
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		} else if (i < LAYER_TEXT1) {
			/* Restore an character. */
			if (saved_layer_file_name[i] != NULL) {
				img = create_image_from_file(CH_DIR,
							     saved_layer_file_name[i]);
				if (img == NULL)
					return false;
			} else {
				img = NULL;
			}
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		} else {
			/* Restore an image. */
			if (saved_layer_file_name[i] != NULL) {
				img = create_image_from_file(CG_DIR,
							     saved_layer_file_name[i]);
				if (img == NULL)
					return false;
			} else {
				img = NULL;
			}
			if (!set_layer_file_name(i, saved_layer_file_name[i]))
				return false;
			set_layer_image(i, img);
		}

		set_layer_position(i, saved_layer_x[i], saved_layer_y[i]);
		set_layer_alpha(i, saved_layer_alpha[i]);
	}
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
