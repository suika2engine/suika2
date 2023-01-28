#include "package.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>

#define CONV_MESSAGE_SIZE 65536

wchar_t wszMessage[CONV_MESSAGE_SIZE];
char szMessage[CONV_MESSAGE_SIZE];

#endif

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

#ifdef _WIN32
int WINAPI wWinMain(
	HINSTANCE hInstance,
	UNUSED(HINSTANCE hPrevInstance),
	UNUSED(LPWSTR lpszCmd),
	int nCmdShow)
{
	return main(2, NULL);
}
#endif

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

#ifdef _WIN32
const wchar_t *conv_utf8_to_utf16(const char *utf8_message)
{
	assert(utf8_message != NULL);

	/* UTF8からUTF16に変換する */
	MultiByteToWideChar(CP_UTF8, 0, utf8_message, -1, wszMessage,
						CONV_MESSAGE_SIZE - 1);

	return wszMessage;
}

const char *conv_utf16_to_utf8(const wchar_t *utf16_message)
{
	assert(utf16_message != NULL);

	/* ワイド文字からUTF-8に変換する */
	WideCharToMultiByte(CP_UTF8, 0, utf16_message, -1, szMessage,
						CONV_MESSAGE_SIZE - 1, NULL, NULL);

	return szMessage;
}
#endif
