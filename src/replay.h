/*
 * Suika2
 * Copyright (C) 2001-2023, Keiich Tabata. All rights reserved.
 */

#ifndef SUIKA_REPLAY_H
#define SUIKA_REPLAY_H

#include "types.h"

#ifdef WIN
bool init_replay(int argc, wchar_t *argv[]);
#else
bool init_replay(int argc, char *argv[]);
#endif

void cleanup_replay(void);
bool capture_input(void);
bool capture_output(void);

#endif
