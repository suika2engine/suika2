/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * Packager
 *
 * [Changes]
 *  - 2016/07/14 Created
 *  - 2022/05/24 Add obfuscation
 *  - 2022/06/14 Move to Suika2 Pro for Creators
 */

#include "suika.h"
#include "package.h"

/* Obfuscation Key */
#include "key.h"

#ifdef WIN
#include <windows.h>
#else
#include <dirent.h>
#endif

/* Max path size */
#define PATH_SIZE		(256)

/* Size of file count which is written at top of an archive */
#define FILE_COUNT_BYTES	(8)

/* Size of file entry */
#define ENTRY_BYTES		(256 + 8 + 8)

/* Directory names */
const char *dir_names[] = {
	"bg", "bgm", "ch", "cg", "cv", "conf", "font", "gui", "rule", "se",
	"txt", "wms", "anime"
};

/* Size of directory names */
#define DIR_COUNT	((int)(sizeof(dir_names) / sizeof(const char *)))

/* File entry */
struct file_entry entry[FILE_ENTRY_SIZE];

/* File count */
static uint64_t file_count;

/* Current processing file's offset in archive file */
static uint64_t offset;

/* Next random number. */
static uint64_t next_random;

/* forward declaration */
static bool get_file_names(const char *base_dir, const char *dir);
static bool get_file_sizes(const char *base_dir);
static bool write_archive_file(const char *base_dir);
static bool write_file_entries(FILE *fp);
static bool write_file_bodies(const char *base_dir, FILE *fp);
static void set_random_seed(uint64_t index);
static char get_next_random(void);

#ifdef WIN
const wchar_t *conv_utf8_to_utf16(const char *utf8_message);
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);
#endif

/*
 * Create package.
 */
bool create_package(const char *base_dir)
{
	int i;

	file_count = 0;
	offset = 0;
	next_random = 0;

	/* Get list of files. */
	for (i = 0; i < DIR_COUNT; i++) {
		if (!get_file_names(base_dir, dir_names[i])) {
			if (strcmp(dir_names[i], "anime") == 0)
				continue;
			return false;
		}
	}

	/* Get all file sizes and decide all offsets in archive. */
	if (!get_file_sizes(base_dir))
		return false;

	/* Write archive file. */
	if (!write_archive_file(base_dir))
		return false;

	return true;
}

#if defined(WIN)
/*
 * For Windows:
 */

static bool get_file_names_recursive(const wchar_t *base_dir, const wchar_t *dir, int depth);

/* Get file list in directory (for Windows) */
static bool get_file_names(const char *base_dir, const char *dir)
{
    wchar_t base_buf[PATH_SIZE];
    wchar_t dir_buf[PATH_SIZE];

    wcsncpy(base_buf, conv_utf8_to_utf16(base_dir), PATH_SIZE - 1);
    base_buf[PATH_SIZE - 1] = L'\0';

    wcsncpy(dir_buf, conv_utf8_to_utf16(dir), PATH_SIZE - 1);
    dir_buf[PATH_SIZE - 1] = L'\0';

    return get_file_names_recursive(base_buf, dir_buf, 0);
}

static bool get_file_names_recursive(const wchar_t *base_dir, const wchar_t *dir, int depth)
{
    HANDLE hFind;
    WIN32_FIND_DATAW wfd;
    wchar_t curdir[PATH_SIZE];
    wchar_t findpath[PATH_SIZE];
    char u8dir[PATH_SIZE];
    char *separator;

    /* Make path. */
    if (wcscmp(base_dir, L"") == 0) {
        /* Make Utf-16 current directory path. */
        _snwprintf(curdir, PATH_SIZE, L"%s", dir);

        /* Make Utf-8 current directory path. */
        snprintf(u8dir, PATH_SIZE, "%s", conv_utf16_to_utf8(dir));
    } else {
        /* Make Utf-16 current directory path. */
        _snwprintf(curdir, PATH_SIZE, L"%s\\%s", base_dir, dir);

        /* Make Utf-8 current directory path. */
        snprintf(u8dir, PATH_SIZE, "%s\\", conv_utf16_to_utf8(base_dir));
        strncat(u8dir, conv_utf16_to_utf8(dir), PATH_SIZE - 1);
        u8dir[PATH_SIZE - 1] = '\0';
    }
    /* Replace '\\' with '/' in u8dir. */
    while ((separator = strstr(u8dir, "\\")) != NULL)
        *separator = '/';

    /* Get directory content. */
    _snwprintf(findpath, PATH_SIZE, L"%s\\*.*", curdir);
    hFind = FindFirstFileW(findpath, &wfd);
    if(hFind == INVALID_HANDLE_VALUE)
    {
        log_dir_not_found(u8dir);
        return false;
    }
    do
    {
        if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
            snprintf(entry[file_count].name, FILE_NAME_SIZE, "%s/%s", u8dir,
                 conv_utf16_to_utf8(wfd.cFileName));
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
            file_count++;
        }
        else if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wfd.cFileName[0] != L'.')
        {
            get_file_names_recursive(curdir, wfd.cFileName, depth + 1);
        }
    } while(FindNextFileW(hFind, &wfd));

    FindClose(hFind);
    return true;
}

#elif defined(OSX)
/*
 * For macOS:
 */

static bool get_file_names_recursive(const char *game_base, const char *base_dir, const char *dir, int depth);

/* Get directory file list (for Mac and Linux) */
static bool get_file_names(const char *base_dir, const char *dir)
{
    bool ret;

    if (!get_file_names_recursive(base_dir, "", dir, 0))
        return false;

    return true;
}

static bool get_file_names_recursive(const char *game_base, const char *base_dir, const char *dir, int depth)
{
    char new_path[1024];
    char query_path[1024];
    struct dirent **names;
    int i, count;
    bool succeeded;

    /* Make a path. */
    if (strcmp(base_dir, "") == 0) {
        snprintf(new_path, sizeof(new_path), "%s", dir);
        snprintf(query_path, sizeof(query_path), "%s/%s", game_base, dir);
    } else {
        snprintf(new_path, sizeof(new_path), "%s/%s", base_dir, dir);
        snprintf(query_path, sizeof(query_path), "%s/%s/%s", game_base, base_dir, dir);
    }

    /* Get directory content. */
    count = scandir(query_path, &names, NULL, alphasort);
    if (count < 0 && depth == 0) {
        log_dir_not_found(dir);
        return false;
    }
    succeeded = true;
    for (i = 0; i < count; i++) {
        if (names[i]->d_name[0] == '.') {
            /* Ignore . and .. (also .*)*/
            continue;
        }
        if (count >= FILE_ENTRY_SIZE) {
            log_too_many_files();
            succeeded = false;
            break;
        }
        if (names[i]->d_type == DT_DIR) {
            if (!get_file_names_recursive(game_base, new_path, names[i]->d_name, depth + 1)) {
                succeeded = false;
                break;
            }
        } else {
            snprintf(entry[file_count].name, FILE_NAME_SIZE,
                     "%s/%s", new_path, names[i]->d_name);
            printf("%s\n", entry[file_count].name);
            file_count++;
        }
    }
    for (i = 0; i < count; i++)
        free(names[i]);
    free(names);
    return succeeded;
}

#else

/*
 * For Linux:
 */

static bool get_file_names_recursive(const char *base_dir, const char *dir, int depth);

/* Get directory file list (for Mac and Linux) */
static bool get_file_names(const char *base_dir, const char *dir)
{
    return get_file_names_recursive(base_dir, dir, 0);
}

static bool get_file_names_recursive(const char *base_dir, const char *dir, int depth)
{
    char new_path[1024];
    struct dirent **names;
    int i, count;
    bool succeeded;

    /* Make a path. */
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
    if (strcmp(base_dir, "") == 0)
        snprintf(new_path, sizeof(new_path), "%s", dir);
    else
        snprintf(new_path, sizeof(new_path), "%s/%s", base_dir, dir);
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif

    /* Get directory content. */
    count = scandir(new_path, &names, NULL, alphasort);
    if (count < 0 && depth == 0) {
        log_dir_not_found(dir);
        return false;
    }
    succeeded = true;
    for (i = 0; i < count; i++) {
        if (names[i]->d_name[0] == '.') {
            /* Ignore . and .. (also .*)*/
            continue;
        }
        if (count >= FILE_ENTRY_SIZE) {
            log_too_many_files();
            succeeded = false;
            break;
        }
        if (names[i]->d_type == DT_DIR) {
            if (!get_file_names_recursive(new_path, names[i]->d_name, depth + 1)) {
                succeeded = false;
                break;
            }
        } else {
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
            snprintf(entry[file_count].name, FILE_NAME_SIZE,
                     "%s/%s", new_path, names[i]->d_name);
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
            printf("%s\n", entry[file_count].name);
            file_count++;
        }
    }
    for (i = 0; i < count; i++)
        free(names[i]);
    free(names);
    return succeeded;
}
#endif

/* Get sizes of each files. */
static bool get_file_sizes(const char *base_dir)
{
	FILE *fp;
	uint64_t i;

	UNUSED_PARAMETER(base_dir);

	/* Get each file size, and calc offsets. */
	offset = FILE_COUNT_BYTES + ENTRY_BYTES * file_count;
	for (i = 0; i < file_count; i++) {
		/*
		 * Make a path and open the file.
		 */
#if defined(WIN)
		/* Make a path on Windows. */
		char *path = strdup(entry[i].name);
		char *slash;
		if (path == NULL) {
			log_memory();
			return false;
		}
		slash = strchr(path, '/');
		if (slash == NULL)
			return false;
		*slash = '\\';
		fp = fopen(path, "rb");
		free(path);
#else
		/* Make path on Mac/Linux. */
		char abspath[1024];
		if (strcmp(base_dir, "") == 0)
			snprintf(abspath, sizeof(abspath), "%s", entry[i].name);
		else
			snprintf(abspath, sizeof(abspath), "%s/%s", base_dir, entry[i].name);
		fp = fopen(abspath, "r");
#endif

		/* Check the fp. */
		if (fp == NULL) {
			log_file_open(entry[i].name);
			return false;
		}

		/* Get the file size. */
		fseek(fp, 0, SEEK_END);
		entry[i].size = (uint64_t)ftell(fp);
		entry[i].offset = offset;
		fclose(fp);

		/* Increment the offset. */
		offset += entry[i].size;
	}
	return true;
}

/* Write archive file. */
static bool write_archive_file(const char *base_dir)
{
	FILE *fp;
	bool success;

#ifdef WIN
	fp = fopen(PACKAGE_FILE, "wb");
	UNUSED_PARAMETER(base_dir);
#else
	char abspath[1024];
	if (strcmp(base_dir, "") == 0)
		snprintf(abspath, sizeof(abspath), "%s", PACKAGE_FILE);
	else
		snprintf(abspath, sizeof(abspath), "%s/%s", base_dir, PACKAGE_FILE);
	fp = fopen(abspath, "w");
#endif
	if (fp == NULL) {
		log_file_open(PACKAGE_FILE);
		return false;
	}

	success = false;
	do {
		if (fwrite(&file_count, sizeof(uint64_t), 1, fp) < 1)
			break;
		if (!write_file_entries(fp))
			break;
		if (!write_file_bodies(base_dir, fp))
			break;
		fclose(fp);
		success = true;
	} while (0);

	if (!success)
		log_file_write(PACKAGE_FILE);

	return true;
}

/* Write file entries. */
static bool write_file_entries(FILE *fp)
{
	char xor[FILE_NAME_SIZE];
	uint64_t i;
	int j;

	for (i = 0; i < file_count; i++) {
		set_random_seed(i);
		for (j = 0; j < FILE_NAME_SIZE; j++)
			xor[j] = entry[i].name[j] ^ get_next_random();

		if (fwrite(xor, FILE_NAME_SIZE, 1, fp) < 1)
			return false;
		if (fwrite(&entry[i].size, sizeof(uint64_t), 1, fp) < 1)
			return false;
		if (fwrite(&entry[i].offset, sizeof(uint64_t), 1, fp) < 1)
			return false;
	}
	return true;
}

/* Write file bodies. */
static bool write_file_bodies(const char *base_dir, FILE *fp)
{
	char buf[8192];
	FILE *fpin;
	uint64_t i;
	size_t len, obf;

	for (i = 0; i < file_count; i++) {
#ifdef WIN
		char *path = strdup(entry[i].name);
		char *slash;
		if (path == NULL) {
			log_memory();
			return false;
		}
		slash = strchr(path, '/');
		if (slash == NULL)
			return false;
		*slash = '\\';
		fpin = fopen(path, "rb");
		UNUSED_PARAMETER(base_dir);
#else
		char abspath[1024];
		if (strcmp(base_dir, "") == 0)
			snprintf(abspath, sizeof(abspath), "%s", entry[i].name);
		else
			snprintf(abspath, sizeof(abspath), "%s/%s", base_dir,
				 entry[i].name);
		fpin = fopen(abspath, "r");
#endif
		if (fpin == NULL) {
			log_file_open(entry[i].name);
			return false;
		}
		set_random_seed(i);
		do  {
			len = fread(buf, 1, sizeof(buf), fpin);
			if (len > 0) {
				for (obf = 0; obf < len; obf++)
					buf[obf] ^= get_next_random();
				if (fwrite(buf, len, 1, fp) < 1) {
					log_file_write(entry[i].name);
					return false;
				}
			}
		} while (len == sizeof(buf));
#ifdef _WIN32
		free(path);
#endif
		fclose(fpin);
	}
	return true;
}

/* Set random seed. */
static void set_random_seed(uint64_t index)
{
	uint64_t i, lsb;

	next_random = OBFUSCATION_KEY;
	for (i = 0; i < index; i++) {
		next_random ^= 0xafcb8f2ff4fff33fULL;
		lsb = next_random >> 63;
		next_random = (next_random << 1) | lsb;
	}
}

/* Get next random number. */
char get_next_random(void)
{
	char ret;

	ret = (char)next_random;

	next_random = (((OBFUSCATION_KEY & 0xff00) * next_random +
			(OBFUSCATION_KEY & 0xff)) % OBFUSCATION_KEY) ^
		      0xfcbfaff8f2f4f3f0;

	return ret;
}
