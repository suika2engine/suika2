/*
 * CLI main code example.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "wms.h"

#define MAX_SCRIPT_SIZE		(4 * 1024 * 1024)

char script[MAX_SCRIPT_SIZE];

static bool hello(struct wms_runtime *rt);

struct wms_ffi_func_tbl ffi_func_tbl[] = {
	{hello, "hello", {"a", "b", "c", "d", NULL}},
};

int main(int argc, char *argv[])
{
	struct wms_runtime *rt;
	FILE *fp;
	size_t len;

	/* Check the command line arguments. */
	if (argc < 2) {
		fprintf(stderr, "Usage: wms <script-file>\n");
		return 1;
	}

	/* Open and read the script file. */
	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open script file %s\n", argv[1]);
		return 1;
	}
	len = fread(script, 1, sizeof(script) - 1, fp);
	if (len == 0 && ferror(fp) != 0) {
		fprintf(stderr, "Could not read the script file.\n");
		return 1;
	}
	script[len] = '\0';
	fclose(fp);

	/* Parse. */
	rt = wms_make_runtime(script);
	if (rt == NULL) {
		fprintf(stderr, "Syntax error at line %d column %d.\n",
			wms_get_parse_error_line(),
			wms_get_parse_error_column());
		return 1;
	}

	/* Register foreign function. (hello()) */
	if (!wms_register_ffi_func_tbl(rt, ffi_func_tbl, 1)) {
		fprintf(stderr, "%s.\n", wms_get_runtime_error_message(rt));
		return 1;
	}

	/* Execute. */
	if (!wms_run(rt)) {
		fprintf(stderr, "%s at line %d.\n",
			wms_get_runtime_error_message(rt),
			wms_get_runtime_error_line(rt));
		return 1;
	}

	/* Free. */
	wms_free_runtime(rt);

	return 0;
}

/* Foreign function example. */
static bool hello(struct wms_runtime *rt)
{
	struct wms_value *a, *b, *c, *d, *ret;
	int a_i;
	double b_f;
	const char *c_s;
	const char *d_s;

	assert(rt != NULL);

	/* Get the argument pointers. */
	if (!wms_get_var_value(rt, "a", &a))
		return false;
	if (!wms_get_var_value(rt, "b", &b))
		return false;
	if (!wms_get_var_value(rt, "c", &c))
		return false;
	if (!wms_get_var_value(rt, "d", &d))
		return false;

	/* Get the argument values. */
	if (!wms_get_int_value(rt, a, &a_i))
		return false;
	if (!wms_get_float_value(rt, b, &b_f))
		return false;
	if (!wms_get_str_value(rt, c, &c_s))
		return false;
	if (!wms_get_array_elem_by_str_for_str(rt, d, "hello", &d_s))
		return false;

	/* Print the values. */
	printf("In FFI hello(): got a = %d\n", a_i);
	printf("In FFI hello(): got b = %f\n", b_f);
	printf("In FFI hello(): got c = %s\n", c_s);
	printf("In FFI hello(): got d[\"hello\"] = %s\n", d_s);

	/* Set the return value. */
	if (!wms_make_array_var(rt, "__return", &ret))
		return false;
	if (!wms_set_array_elem_by_str_for_str(rt, ret, "hello", "bonjour"))
		return false;

	return true;
}

/*
 * Printer. (for print() intrinsic)
 */
int wms_printf(const char *s, ...)
{
	va_list ap;
	int ret;

	va_start(ap, s);
	ret = vprintf(s, ap);
	va_end(ap);

	return ret;
}

/*
 * Reader. (for readline() intrinsic)
 */
int wms_readline(char *buf, size_t len)
{
	int ret;
	if (fgets(buf, len, stdin) == NULL)
		return 0;
	ret = strlen(buf);
	if (ret > 0)
		buf[ret - 1] = '\0';
	return ret - 1;
}
