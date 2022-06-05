/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include "key.h"

/*

Archive file design

struct header {
    u64 file_count;
    struct entry {
        u8  file_name[256]; // This is encrypted
        u64 file_size;
        u64 file_offset;
    } [file_count];
};
u8 file_body[file_count][]; // These are encrypted

*/

/* Maximum number of file entries */
#define ENTRY_SIZE		(65536)

/* Size of file name */
#define FILE_NAME_SIZE		(256)

/* Size of file count which is written at top of an archive */
#define FILE_COUNT_BYTES	(8)

/* Size of file entry */
#define ENTRY_BYTES		(256 + 8 + 8)

/* Package file name */
#define PACKAGE_FILE_NAME	"data01.arc"

/* Directory names */
const char *dir_names[] = {
	"bg", "bgm", "ch", "cg", "cv", "conf", "font", "se", "txt"
};

/* Size of directory names */
#define DIR_COUNT	(sizeof(dir_names) / sizeof(const char *))

/* File entry */
struct entry {
	/* File name */
	char name[FILE_NAME_SIZE];

	/* File size */
	uint64_t size;

	/* File offset in archive file */
	uint64_t offset;
} entry[ENTRY_SIZE];

/* File count */
uint64_t file_count;

/* Current processing file's offset in archive file */
uint64_t offset;

/* Next random number. */
uint64_t next_random;

/* forward declaration */
bool write_file_entries(FILE *fp);
bool write_file_bodies(FILE *fp);
void set_random_seed(uint64_t index);
char get_next_random(void);

#ifdef _WIN32
/*
 * Get file list in directory(for Windows)
 */
bool get_file_names(const char *dir)
{
    char path[256];
    HANDLE hFind;
    WIN32_FIND_DATA wfd;

    /* Get directory content. */
    snprintf(path, sizeof(path), "%s\\*.*", dir);
    hFind = FindFirstFile(path, &wfd);
    if(hFind == INVALID_HANDLE_VALUE)
    {
        printf("Directory %s not found.\n", dir);
        return false;
    }
    do
    {
        if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            snprintf(entry[file_count].name, FILE_NAME_SIZE,
                     "%s/%s", dir, wfd.cFileName);
            printf("  %s\n", entry[file_count].name);
            file_count++;
	}
    } while (FindNextFile(hFind, &wfd));

    FindClose(hFind);
    return true;
}
#else
/*
 * Get directory file list(for UNIX)
 */
bool get_file_names(const char *dir)
{
	char path[FILE_NAME_SIZE];
	struct dirent **names;
	int i, count;

	/* Get directory content. */
	count = scandir(dir, &names, NULL, alphasort);
	if (count < 0) {
		printf("Directory %s not found.\n", dir);
		return false;
	}
	if (count > ENTRY_SIZE) {
		printf("Too many files.");
		return false;	/* Intentional memory leak. */
	}
	for (i = 0; i < count; i++) {
		if (names[i]->d_name[0] == '.')
			continue;

		snprintf(entry[file_count].name, FILE_NAME_SIZE,
			 "%s/%s", dir, names[i]->d_name);

		printf("  %s\n", entry[file_count].name);
		free(names[i]);
		file_count++;
	}
	free(names);
	return true;
}
#endif

/*
 * Get sizes of each files.
 */
bool get_file_sizes(void)
{
	uint64_t i;
	FILE *fp;

	/* Get each file size, and calc offsets. */
	offset = FILE_COUNT_BYTES + ENTRY_BYTES * file_count;
	for (i = 0; i < file_count; i++) {
#ifdef _WIN32
		char *path = strdup(entry[i].name);
		*strchr(path, '/') = '\\';
		fp = fopen(path, "rb");
#else
		fp = fopen(entry[i].name, "r");
#endif
		if (fp == NULL) {
			printf("Can't open file %s\n", entry[i].name);
			return false;
		}
		fseek(fp, 0, SEEK_END);
		entry[i].size = ftell(fp);
		entry[i].offset = offset;
		fclose(fp);
#ifdef _WIN32
		free(path);
#endif
		offset += entry[i].size;
	}
	return true;
}

/*
 * Write archive file.
 */
bool write_archive_file(void)
{
	FILE *fp;
	bool success;

	fp = fopen(PACKAGE_FILE_NAME, "wb");
	if (fp == NULL) {
		printf("Can't open %s\n", PACKAGE_FILE_NAME);
		return false;
	}

	success = false;
	do {
		if (fwrite(&file_count, sizeof(uint64_t), 1, fp) < 1)
			break;
		if (!write_file_entries(fp))
			break;
		if (!write_file_bodies(fp))
			break;
	} while (0);

	return true;
}

/* Write file entries. */
bool write_file_entries(FILE *fp)
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

/*
 * Write file bodies.
 */
bool write_file_bodies(FILE *fp)
{
	char buf[8192];
	FILE *fpin;
	uint64_t i;
	size_t len, obf;

	for (i = 0; i < file_count; i++) {
#ifdef _WIN32
		char *path = strdup(entry[i].name);
		*strchr(path, '/') = '\\';
		fpin = fopen(path, "rb");
#else
		fpin = fopen(entry[i].name, "rb");
#endif
		if (fpin == NULL) {
			printf("Can't open %s\n", entry[i].name);
			return false;
		}
		set_random_seed(i);
		do  {
			len = fread(buf, 1, sizeof(buf), fpin);
			if (len > 0) {
				for (obf = 0; obf < len; obf++)
					buf[obf] ^= get_next_random();
				if (fwrite(buf, len, 1, fp) < 1)
					return false;
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
void set_random_seed(uint64_t index)
{
	uint64_t i, lsb;

	next_random = ENCRYPTION_KEY;
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

	next_random = (((ENCRYPTION_KEY & 0xff00) * next_random +
			(ENCRYPTION_KEY & 0xff)) % ENCRYPTION_KEY) ^
		      0xfcbfaff8f2f4f3f0;

	return ret;
}

int main(int argc, char *argv[])
{
	int i;

	printf("Hello, this is Suika2's packager.\n");

	/* Get list of files. */
	printf("Searching files...\n");
	for (i = 0; i < DIR_COUNT; i++)
		if (!get_file_names(dir_names[i]))
			return 1;

	/* Get all file sizes and decide all offsets in archive. */
	printf("Checking file sizes...\n");
	if (!get_file_sizes())
		return 1;

	/* Write archive file. */
	printf("Writing archive files...\n");
	if (!write_archive_file())
		return 1;

	printf("Done.\n");
	return 0;
}
