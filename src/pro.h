/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * A HAL API extension for Suika2 Pro
 *
 * [History]
 *  - 2023-12-08 Created.
 *  - 2024-02-08 Removed the old APIs and moved to editor version on all the platforms.
 */

#ifndef SUIKA_PRO_H
#define SUIKA_PRO_H

#if defined(USE_EDITOR)

/*
 * Return whether the "continue" botton is pressed.
 */
bool is_continue_pushed(void);

/*
 * Return whether the "next" button is pressed.
 */
bool is_next_pushed(void);

/*
 * Return whether the "stop" button is pressed.
 */
bool is_stop_pushed(void);

/*
 * Return whether the "open" button is pressed.
 */
bool is_script_opened(void);

/*
 * Return a script file name when the "open" button is pressed.
 */
const char *get_opened_script(void);

/*
 * Return whether the "execution line number" is changed.
 */
bool is_exec_line_changed(void);

/*
 * Return the "execution line number" if it is changed.
 */
int get_changed_exec_line(void);

/*
 * Update UI elements when the running state is changed.
 */
void on_change_running_state(bool running, bool request_stop);

/*
 * Update UI elements when the main engine changes the script to be executed.
 */
void on_load_script(void);

/*
 * Update UI elements when the main engine changes the command position to be executed.
 */
void on_change_position(void);

/*
 * Update UI elements when the main engine changes variables.
 */
void on_update_variable(void);

#endif /* defined(USE_EDITOR) */

#endif /* SUIKA_PRO_H */
