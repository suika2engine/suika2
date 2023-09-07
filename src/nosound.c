/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * Dummy Sound Module
 *
 * [Changes]
 *  - 2023/09/07 Created.
 */

#include "suika.h"

/*
 * A HAL (platform.h API) implementation
 */

bool play_sound(int stream, struct wave *w)
{
	return true;
}

bool stop_sound(int stream)
{
	return true;
}

bool set_sound_volume(int stream, float vol)
{
	return true;
}

bool is_sound_finished(int stream)
{
	return true;
}
