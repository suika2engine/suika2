/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * A header to absorb compiler differences.
 *  - We support:
 *    - GCC up to 11
 *    - LLVM/Clang up to 14
 *    - MSVC (From Visual C++ 6.0 to Visual Studio 2023)
 */

#ifndef SUIKA_TYPES_H
#define SUIKA_TYPES_H

/*
 * Use NULL macro.
 */
#include <stddef.h>

/*
 * Define a macro that indicates a platform we are compiling to.
 *  - WIN: for Win32 and Win64
 *  - OSX: for macOS
 *  - ANDROID: for Android
 *  - IOS: for iOS
 *  - EM: for Web (Emscripten)
 */
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define IOS
#else
#define OSX
#endif
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
#elif defined(__EMSCRIPTEN__)
#define EM
#elif !defined(SWITCH)
#define LINUX
#endif

/*
 * For GCC
 */
#if defined(__GNUC__) && !defined(__llvm__)

/*
 * Use uint8_t, uint16_t, uint32_t and their signed ones.
 */
#include <stdint.h>

/*
 * Use bool.
 *  - If this header is included from a .cc or .cpp file, avoid including stdbool.h
 */
#ifndef __cplusplus
#include <stdbool.h>
#endif

/*
 * Define a macro that indicates a target architecture.
 *  - X86 for ia32 (x86)
 *  - X86_64 for amd64 (x86_64)
 */
#if defined(__i386__) && !defined(__x86_64__)
#define X86
#elif defined(__x86_64__)
#define X86_64
#endif

/*
 * Decide wehether to use dynamic dispatching of the fastest SSE/AVX extension.
 *  - SSE_VERSIONING for dispatching of SSE/AVX codes
 */
#if defined(X86) || defined(X86_64)
#if !defined(ANDROID) && !defined(IOS) && !defined(NO_SSE_VERSIONING)
#define SSE_VERSIONING
#endif
#else
#undef SSE_VERSIONING
#endif

/*
 * Non-C89 keyword wrappers
 */

/* Inline function. */
#define INLINE			__inline

/* No pointer aliasing. */
#define RESTRICT	 	__restrict

/* Suppress unused warnings. */
#define UNUSED(x)		x __attribute__((unused))
#define UNUSED_PARAMETER(x)	(void)(x)

/* Declare a variable with memory alignment. */
#define ALIGN_DECL(n, cdecl)	cdecl __attribute__((aligned(n)))

/* Utf-8 string literal for non-ASCII strings. */
#define U8(s)			s

/* Utf-32 character literal for non-ASCII characters. */
#define U32_C(s)		U##s

/* Aligned malloc() for mingw excluding mingw-w64. */
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc	__mingw_aligned_malloc
#define _aligned_free	__mingw_aligned_free
#endif

#endif /* End of GCC */

/*
 * For LLVM
 */
#if defined(__llvm__)

/*
 * Use uint8_t, uint16_t, uint32_t and their signed ones.
 */
#include <stdint.h>

/*
 * Use bool.
 *  - If this header is included from a .cc or .cpp file, avoid including stdbool.h
 */
#ifndef __cplusplus
#include <stdbool.h>
#endif

/*
 * Define a macro that indicates a target architecture.
 *  - X86 for ia32 (x86)
 *  - X86_64 for amd64 (x86_64)
 */
#if defined(__i386__) && !defined(__x86_64__)
#define X86
#elif defined(__x86_64__)
#define X86_64
#endif

/*
 * Decide wehether to use dynamic dispatching of the fastest SSE/AVX extension.
 *  - SSE_VERSIONING for dispatching of SSE/AVX codes
 */
#if defined(X86) || defined(X86_64)
/* Avoid Android NDK for now due to CMake compilation option issue. */
#if !defined(ANDROID) && !defined(NO_SSE_VERSIONING)
#define SSE_VERSIONING
#endif
#endif

/*
 * Non-C89 keyword wrappers
 */

/* Inline function. */
#define INLINE			__inline

/* No pointer aliasing. */
#define RESTRICT	 	__restrict

/* Suppress unused warnings. */
#define UNUSED(x)		x __attribute__((unused))
#define UNUSED_PARAMETER(x)	(void)(x)

/* Declare a variable with memory alignment. */
#define ALIGN_DECL(n, cdecl)	cdecl __attribute__((aligned(n)))

/* Utf-8 string literal for non-ASCII strings. */
#define U8(s)			s

/* Utf-32 character literal for non-ASCII characters. */
#define U32_C(s)		U##s

#endif	/* End of LLVM */

/*
 * For MSVC
 */
#ifdef _MSC_VER

/*
 * Use uint8_t, uint16_t, uint32_t and their signed ones.
 */
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
typedef unsigned long uintptr_t;    /* VC2010以前では64bitコンパイルしないこと */
#endif

/*
 * Use bool.
 *  - If this header is included from a .cc or .cpp file, avoid including stdbool.h
 */
#if _MSC_VER >= 1800 /* VC2013 */
#ifndef __cplusplus
#include <stdbool.h>
#endif
#else
#ifndef __cplusplus
#define bool	int
#define false	(0)
#define true	(1)
#endif
#endif

/*
 * Define a macro that indicates a target architecture.
 *  - X86 for ia32 (x86)
 *  - X86_64 for amd64 (x86_64)
 */
#if defined(_M_IX86)
#define X86
#elif defined(_M_X64)
#define X86_64
#endif

/*
 * Decide wehether to use dynamic dispatching of the fastest SSE/AVX extension.
 *  - SSE_VERSIONING for dispatching of SSE/AVX codes
 */
#if _MSCVER >= 1920 /* VC2019 */
#if defined(X86) || defined(X86_64)
#define SSE_VERSIONING
#endif
#endif

/*
 * Non-C89 keyword wrappers
 */

/* Inline function. */
#define INLINE			__inline

/* No pointer aliasing. */
#if _MSC_VER >= 1400		/* VC2005 */
#define RESTRICT		__restrict
#else
#define RESTRICT
#endif

/* Suppress unused warnings. */
#define UNUSED(x)		x
#define UNUSED_PARAMETER(x)	(void)(x)

/* Declare a variable with memory alignment. */
#define ALIGN_DECL(n, cdecl)	__declspec(align(n) cdecl)

/* Utf-8 string literal for non-ASCII strings. */
#define U8(s)			u8##s

/* Utf-8 string literal for non-ASCII strings. */
#define U32_C(s)		U##s

/*
 * POSIX libc functions
 */
#define strdup			_strdup
#if _MSC_VER <= 1600		/* VC2010 */
#define vsnprintf		_vsnprintf
#define snprintf		_snprintf
#endif

#endif /* End of MSVC */

/*
 * Additional staff. (platform independent)
 */

/*
 * Milliseconds lap timers - we call it stop watches.
 */
typedef uint64_t stop_watch_t;

#endif
