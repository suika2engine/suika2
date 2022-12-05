#include "package.h"
#include <stdio.h>
#include <stdarg.h>

/* Force English output. */
int conf_i18n = 1;

int main(int argc, char *argv[])
{
	/* Create a package. */
	if (!create_package(".")) {
		printf("Failed.\n");
		return 1;
	}

	printf("Suceeded.\n");
	return 0;
}

/*
 * Logging Functions
 */

bool log_error(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	printf("\n");
	return true;
}

bool log_warn(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	printf("\n");
	return true;
}

bool log_info(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	printf("\n");
	return true;
}

const char *conv_utf8_to_native(const char *utf8_message)
{
	/* stub */
	return utf8_message;
}

/*
 * Stub Functions
 */

const char *get_script_file_name(void)
{
	return "";
}

int get_line_num(void)
{
	return 0;
}

const char *get_line_string(void)
{
	return "";
}

int get_command_index(void)
{
	return 0;
}

void dbg_set_error_state(void)
{
}

void set_error_command(int index, char *text)
{
}

const char *get_system_locale(void)
{
	return "other";
}
