/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

#ifndef SUIKA_TYPES_H
#define SUIKA_TYPES_H

#include <stddef.h>

/*
 * Target
 */
#if defined(__APPLE__)
#define OSX
#elif defined(_WIN32)
#define WIN
#elif defined(__ANDROID__)
#ifndef ANDROID
#define ANDROID
#endif
#elif defined(__NetBSD__)
#define NETBSD
#elif defined(__FreeBSD__)
#define FREEBSD
#else
#define LINUX
#endif

/*
 * For GCC
 */
#if defined(__GNUC__) && !defined(__llvm__)

/* stdint.h */
#include <stdint.h>

/* stdbool.h */
#include <stdbool.h>

/* アーキテクチャ */
#if defined(__i386__) && !defined(__x86_64__)
#define X86
#elif defined(__x86_64__)
#define X86_64
#endif

/* x86/x86_64のSSEバージョニングを行うか */
#if (defined(__i386__) || defined(__x86_64__)) && !defined(ANDROID)
#define SSE_VERSIONING
#endif

/* キーワード */
#define INLINE			__inline
#define RESTRICT	 	__restrict
#define UNUSED(x)		x __attribute__((unused))
#define UNUSED_PARAMETER(x)	(void)(x)
#define ALIGN_DECL(n, cdecl)	cdecl __attribute__((aligned(n)))

/* 関数 */
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc	__mingw_aligned_malloc
#define _aligned_free	__mingw_aligned_free
#endif

#endif /* GCC */

/*
 * For LLVM
 */
#if defined(__llvm__)

/* stdint.h */
#include <stdint.h>

/* stdbool.h */
#include <stdbool.h>

/* アーキテクチャ */
#if defined(__i386__) && !defined(__x86_64__)
#define X86
#elif defined(__x86_64__)
#define X86_64
#endif

/* x86/x86_64のSSEバージョニングを行うか */
#if (defined(__i386__) || defined(__x86_64__)) && !defined(ANDROID) &&\
    !defined(XCODE)
#define SSE_VERSIONING
#endif

/* キーワード */
#define INLINE			__inline
#define RESTRICT 		__restrict
#define UNUSED(x)		x __attribute__((unused))
#define UNUSED_PARAMETER(x)	(void)(x)
#define ALIGN_DECL(n, cdecl)	cdecl __attribute__((aligned(n)))

/* 関数 */

#endif	/* LLVM */

/*
 * For MSVC
 */
#ifdef _MSC_VER

/* stdint.h */
#if _MSC_VER >= 1600 /* VC2010 */
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef long intptr_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef unsigned long uintptr_t;
#endif

/* stdbool.h */
#if _MSC_VER >= 1800 /* VC2013 */
#include <stdbool.h>
#else
#define bool	int
#define false	(0)
#define true	(1)
#endif

/* アーキテクチャ */
#if defined(_M_IX86)
#define X86
#elif defined(_M_X64)
#define X86_64
#endif

/* x86/x86_64のSSEバージョニングを行うか */
#if _MSCVER >= 1920 /* VC2019 */
#if defined(_M_IX86) || defined(_M_X64)
#define SSE_VERSIONING
#endif
#endif

/* キーワード */
#define INLINE			__inline
#define UNUSED(x)		x
#define UNUSED_PARAMETER(x)	(void)(x)
#if _MSC_VER >= 1400		/* VC2005 */
#define RESTRICT		__restrict
#else
#define RESTRICT
#endif
#define ALIGN_DECL(n, cdecl)	__declspec(align(n) cdecl)

/* 関数 */
#define strdup			_strdup
#if _MSC_VER <= 1600		/* VC2010 */
#define vsnprintf		_vsnprintf
#define snprintf		_snprintf
#endif

#endif /* MSVC */

/* ストップウォッチ型(ミリ秒) */
typedef uint64_t stop_watch_t;

#endif
