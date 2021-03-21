/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-11 作成
 */

#include "suika.h"
#include "x86.h"

#ifdef SSE_VERSIONING

#ifdef WIN
#include <windows.h>
#endif

bool has_avx512;
bool has_avx2;
bool has_avx;
bool has_sse42;
bool has_sse41;
bool has_sse3;
bool has_sse2;
bool has_sse;

static void asm_cpuid(uint32_t fn, uint32_t* eax, uint32_t* ebx, uint32_t* ecx,
		      uint32_t* edx);
static uint32_t asm_xgetbv(void);

#ifdef WIN
static void clear_sse_flags_by_os_version(void);
#endif

/*
 * For gcc
 */
#ifdef __GNUC__

static void asm_cpuid(uint32_t fn, uint32_t* eax, uint32_t* ebx, uint32_t* ecx,
	uint32_t* edx)
{
	asm("cpuid" :
	    "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) :
	    "a"(fn), "c"(0));
}

static uint32_t asm_xgetbv(void)
{
	uint32_t a;
	asm("xgetbv": "=a"(a) : "c"(0));
	return a;
}

#endif /* __GNUC__ */

/*
 * For Visual Studio
 */
#ifdef _MSC_VER

#include <intrin.h>

static void asm_cpuid(uint32_t fn, uint32_t* eax, uint32_t* ebx, uint32_t* ecx,
	uint32_t* edx)
{
	uint32_t regs[4];
	__cpuid(regs, fn);
	*eax = regs[0];
	*ebx = regs[1];
	*ecx = regs[2];
	*edx = regs[3];
}

uint32_t asm_xgetbv(void)
{
	return (uint32_t)_xgetbv(0);
}

#endif /* _MSC_VER */

/*
 * ベクトル命令の対応状況を調べる
 */
void x86_check_cpuid_flags(void)
{
	uint32_t a, b, c, d;
	bool has_osxsave;

	/* CPUID命令でベクトル命令のサポートを調べる */
	asm_cpuid(7, &a, &b, &c, &d);
	has_avx512 = b & (1 << 16);
	has_avx2 = b & (1 << 5);
	asm_cpuid(1, &a, &b, &c, &d);
	has_avx = c & (1 << 28);
	has_osxsave = c & (1 << 27);
	has_sse42 = c & (1 << 20);
	has_sse41 = c & (1 << 19);
	has_sse3 = c & 1;
	has_sse2 = d & (1 << 26);
	has_sse = d & (1 << 25);

#ifdef WIN
	/* WindowsのバージョンによってSSEを無効化する */
	clear_sse_flags_by_os_version();
#endif

	/* XGETBV命令がOSでサポートされている場合 */
	if (has_osxsave) {
		a = asm_xgetbv();

		/* ZMMが保存されない場合 */
		if (has_avx512 && (a & 0xc0) != 0xc0) {
			/* OS disables AVX512F. */
			has_avx512 = false;
		}

		/* YMMが保存されない場合 */
		if ((has_avx2 || has_avx) && (a & 0x04) != 0x04) {
			/* OS doesn't support AVX/AVX2. */
			has_avx = false;
			has_avx2 = false;
		}
	} else {
		/* OS doesn't support XGETBV. */
		has_avx512 = false;
		has_avx2 = false;
		has_avx = false;
	}

	/* For now, disable AVX512 because I can't test it. */
	has_avx512 = false;
}

#ifdef WIN
/*
 * Windowsのバージョンでベクトル命令を無効化する
 */
static void clear_sse_flags_by_os_version(void)
{
	OSVERSIONINFO vi;
	DWORD dwMajor, dwMinor, dwBuild;

	ZeroMemory(&vi, sizeof(OSVERSIONINFO));
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	/* Windowsのバージョンを取得する */
	if (!GetVersionEx(&vi)) {
		has_avx512 = false;
		has_avx2 = false;
		has_avx = false;
		has_sse42 = false;
		has_sse41 = false;
		has_sse3 = false;
		has_sse2 = false;
		has_sse = false;
		return;
	}
	dwMajor = vi.dwMajorVersion;
	dwMinor = vi.dwMinorVersion;
	dwBuild = vi.dwBuildNumber;

	/* Windows 98 SEより前の場合、全てのSIMD命令を無効にする */
	if (dwMajor < 4 ||
	    (dwMajor == 4 && dwMinor < 10) ||
	    (dwMajor == 4 && dwMinor == 10 && dwBuild < 2222)) {
		has_avx512 = false;
		has_avx2 = false;
		has_avx = false;
		has_sse42 = false;
		has_sse41 = false;
		has_sse3 = false;
		has_sse2 = false;
		has_sse = false;
	}
}
#endif

#endif /* SSE_VERSIONING */
