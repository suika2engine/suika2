/*
 * Suika2
 * Copyright (C) 2001-2023, Keiich Tabata. All rights reserved.
 */

#ifndef SUIKA_REPLAY_H
#define SUIKA_REPLAY_H

#include "types.h"

/*
 * CSV header for capture and replay
 */
#define CSV_HEADER "time,PNG,X,Y,left,right,lclick,rclick,return,space,escape,up,down,pageup,pagedown,control\n"

/*
 * The replay module interface.
 */
bool init_replay(int argc, char *argv[]);
void cleanup_replay(void);
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

/*
 * HAL: A wrapper for fopen().
 */
FILE *fopen_wrapper(const char *dir, const char *file, const char *mode);

#endif
