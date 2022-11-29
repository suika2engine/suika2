/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-11 作成
 */

#ifndef X86_H
#define X86_H

#include "types.h"

/*
 * SSEバージョニングを行う場合のみ
 */
#ifdef SSE_VERSIONING

/* 256bit境界にする */
#define SSE_ALIGN	(32)

/* CPUIDフラグ */
extern bool has_avx512;
extern bool has_avx2;
extern bool has_avx;
extern bool has_sse42;
extern bool has_sse41;
extern bool has_sse3;
extern bool has_sse2;
extern bool has_sse;

/* CPUID命令でフラグを取得する */
void x86_check_cpuid_flags(void);

#endif	/* SSE_VERSIONING */

#endif
