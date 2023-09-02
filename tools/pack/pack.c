#include "package.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>

int WINAPI wWinMain(
	HINSTANCE hInstance,
	UNUSED(HINSTANCE hPrevInstance),
	UNUSED(LPWSTR lpszCmd),
	int nCmdShow)
{
	int main(int, char *[]);
	return main(2, NULL);
}
#endif

int main(int argc, char *argv[])
{
	printf("Hello, this is a Suika2 packager!\n");

	/* Create a package. */
	if (!create_package(".")) {
		printf("Failed.\n");
		return 1;
	}

	printf("Suceeded.\n");
	return 0;
}

#ifdef _WIN32
/*
 * Conversions between UTF-8 and UTF-16.
 */

#define CONV_MESSAGE_SIZE 65536

static wchar_t wszMessage[CONV_MESSAGE_SIZE];
static char szMessage[CONV_MESSAGE_SIZE];

const wchar_t *conv_utf8_to_utf16(const char *utf8_message)
{
	assert(utf8_message != NULL);
	MultiByteToWideChar(CP_UTF8, 0, utf8_message, -1, wszMessage,
			    CONV_MESSAGE_SIZE - 1);
	return wszMessage;
}

const char *conv_utf16_to_utf8(const wchar_t *utf16_message)
{
	assert(utf16_message != NULL);
	WideCharToMultiByte(CP_UTF8, 0, utf16_message, -1, szMessage,
			    CONV_MESSAGE_SIZE - 1, NULL, NULL);
	return szMessage;
}
#endif

/*
 * Stub for conf.c
 */

/* Force English output. */
int conf_i18n = 1;

/*
 * Stub for platform.c
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
	return utf8_message;
}

const char *get_system_locale(void)
{
	return "other";
}

/*
 * Stub for script.c
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

void translate_failed_command_to_message(int index)
{
}

/*
 * Stub for main.c
 */

void dbg_set_error_state(void)
{
}
