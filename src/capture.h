/*
 * Suika2
 * Copyright (C) 2001-2023, Keiich Tabata. All rights reserved.
 */

#ifndef SUIKA_CAPTURE_H
#define SUIKA_CAPTURE_H

#include "types.h"

/*
 * CSV header for capture and replay
 */
#define CSV_HEADER "time,PNG,X,Y,left,right,lclick,rclick,return,space,escape,up,down,pageup,pagedown,control\n"

/*
 * The capture module interface.
 */
bool init_capture(void);
void cleanup_capture(void);
bool capture_input(void);
bool capture_output(void);

/*
 * HAL: Removes and re-create an output directory.
 */
bool reconstruct_dir(const char *dir);

/*
 * HAL: Gets a clock in a millisecond precise.
 */
uint64_t get_tick_count64(void);

#endif
