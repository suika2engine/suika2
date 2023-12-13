/*
 * Suika2
 * Copyright (c) 2001-2023, Keiichi Tabata. All rights reserved.
 */

#ifndef WMS_H
#define WMS_H

#include <stdbool.h>

/*
 * Runtime Structure
 */
struct wms_runtime;

/*
 * Execution
 */

/* Parse script and get runtime. */
struct wms_runtime *wms_make_runtime(const char *script);

/* Get parse error line.  */
int wms_get_parse_error_line(void);

/* Get parse error column. */
int wms_get_parse_error_column(void);

/* Execute main function. */
bool wms_run(struct wms_runtime *rt);

/* Get runtime error line. */
int wms_get_runtime_error_line(struct wms_runtime *rt);

/* Get runtime error message. */
const char *wms_get_runtime_error_message(struct wms_runtime *rt);

/* Cleanup runtime. */
void wms_free_runtime(struct wms_runtime *rt);

/* Must be implemented by the main code. */
int wms_printf(const char *s, ...);

/* Must be implemented by the main code. */
int wms_readline(char *buf, size_t len);

/*
 * Foreign Function Interface
 */

/* Value type. */
struct wms_value;

/* Array element type. */
struct wms_array_element;

/* Pointer to foreign function. */
typedef bool (*wms_ffi_func_ptr)(struct wms_runtime *rt);

/* FFI function table. */
struct wms_ffi_func_tbl {
	wms_ffi_func_ptr func_ptr;
	const char *func_name;
	const char *param_name[16];
};

/* Register a foreign function. */
bool wms_register_ffi_func_tbl(struct wms_runtime *rt, struct wms_ffi_func_tbl *ffi_func_tbl, int count);

/* Get the value of value. */
bool wms_get_var_value(struct wms_runtime *rt, const char *symbol, struct wms_value **ret);

/* Get the type of `struct wms_value`. */
bool wms_is_int(struct wms_runtime *rt, struct wms_value *val);
bool wms_is_float(struct wms_runtime *rt, struct wms_value *val);
bool wms_is_str(struct wms_runtime *rt, struct wms_value *val);
bool wms_is_array(struct wms_runtime *rt, struct wms_value *val);

/* Get the value of `struct wms_value`. */
bool wms_get_int_value(struct wms_runtime *rt, struct wms_value *val, int *ret);
bool wms_get_float_value(struct wms_runtime *rt, struct wms_value *val, double *ret);
bool wms_get_str_value(struct wms_runtime *rt, struct wms_value *val, const char **ret);

/* Array element traverse. */
struct wms_array_elem *wms_get_first_array_elem(struct wms_runtime *rt, struct wms_value *array);
struct wms_array_elem *wms_get_next_array_elem(struct wms_runtime *rt, struct wms_array_elem *prev);

/* Set the value of a variable. */
bool wms_make_int_var(struct wms_runtime *rt, const char *symbol, int val, struct wms_value **ret);
bool wms_make_float_var(struct wms_runtime *rt, const char *symbol, double val, struct wms_value **ret);
bool wms_make_str_var(struct wms_runtime *rt, const char *symbol, const char *val, struct wms_value **ret);
bool wms_make_array_var(struct wms_runtime *rt, const char *symbol, struct wms_value **ret);

/* Getters for array element. */
bool wms_get_array_elem(struct wms_runtime *rt, struct wms_value *array, struct wms_value *index, struct wms_value **ret);
bool wms_get_array_elem_by_int_for_int(struct wms_runtime *rt, struct wms_value *array, int index, int *ret);
bool wms_get_array_elem_by_int_for_float(struct wms_runtime *rt, struct wms_value *array, int index, double *ret);
bool wms_get_array_elem_by_int_for_str(struct wms_runtime *rt, struct wms_value *array, int index, const char **ret);
bool wms_get_array_elem_by_int_for_array(struct wms_runtime *rt, struct wms_value *array, int index, struct wms_value **ret);
bool wms_get_array_elem_by_float_for_int(struct wms_runtime *rt, struct wms_value *array, double index, int *ret);
bool wms_get_array_elem_by_float_for_float(struct wms_runtime *rt, struct wms_value *array, double index, double *ret);
bool wms_get_array_elem_by_float_for_str(struct wms_runtime *rt, struct wms_value *array, double index, const char **ret);
bool wms_get_array_elem_by_float_for_array(struct wms_runtime *rt, struct wms_value *array, double index, struct wms_value **ret);
bool wms_get_array_elem_by_str_for_int(struct wms_runtime *rt, struct wms_value *array, const char *index, int *ret);
bool wms_get_array_elem_by_str_for_float(struct wms_runtime *rt, struct wms_value *array, const char *index, double *ret);
bool wms_get_array_elem_by_str_for_str(struct wms_runtime *rt, struct wms_value *array, const char *index, const char **ret);
bool wms_get_array_elem_by_str_for_array(struct wms_runtime *rt, struct wms_value *array, const char *index, struct wms_value **ret);

/* Setters for array element. */
bool wms_set_array_elem(struct wms_runtime *rt, struct wms_value *array, struct wms_value *index, struct wms_value *val);
bool wms_set_array_elem_by_int_for_int(struct wms_runtime *rt, struct wms_value *array, int index, int val);
bool wms_set_array_elem_by_int_for_float(struct wms_runtime *rt, struct wms_value *array, int index, double val);
bool wms_set_array_elem_by_int_for_str(struct wms_runtime *rt, struct wms_value *array, int index, const char *val);
bool wms_set_array_elem_by_int_for_array(struct wms_runtime *rt, struct wms_value *array, int index, struct wms_value *val);
bool wms_set_array_elem_by_float_for_int(struct wms_runtime *rt, struct wms_value *array, double index, int val);
bool wms_set_array_elem_by_float_for_float(struct wms_runtime *rt, struct wms_value *array, double index, double val);
bool wms_set_array_elem_by_float_for_str(struct wms_runtime *rt, struct wms_value *array, double index, const char *val);
bool wms_set_array_elem_by_float_for_array(struct wms_runtime *rt, struct wms_value *array, double index, struct wms_value *val);
bool wms_set_array_elem_by_str_for_int(struct wms_runtime *rt, struct wms_value *array, const char *index, int val);
bool wms_set_array_elem_by_str_for_float(struct wms_runtime *rt, struct wms_value *array, const char *index, double val);
bool wms_set_array_elem_by_str_for_str(struct wms_runtime *rt, struct wms_value *array, const char *index, const char *val);
bool wms_set_array_elem_by_str_for_array(struct wms_runtime *rt, struct wms_value *array, const char *index, struct wms_value *val);

#endif
