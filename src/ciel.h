/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * The Ciel Direction System
 *
 * [Changes]
 *  - 2024/03/27 Created.
 */

#ifndef SUIKA_CIEL_H
#define SUIKA_CIEL_H

#include "file.h"

bool ciel_serialize_hook(struct wfile *wf);
bool ciel_deserialize_hook(struct rfile *rf);

#endif
