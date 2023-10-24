/*
 * Suika2
 * Copyright (C) 2001-2023, Keiich Tabata. All rights reserved.
 */

#ifndef SUIKA_CAPTURE_H
#define SUIKA_CAPTURE_H

#include "types.h"

bool init_capture(void);
void cleanup_capture(void);
bool capture_input(void);
bool capture_output(void);

#endif
