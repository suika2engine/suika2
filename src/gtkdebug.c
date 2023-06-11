/* -*- c-basic-offset: 2; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * GTK Debug Window Module
 *
 * [Changes]
 *  4th May 2023 Created by Keiichi Tabata.
 */

#include "suika.h"

#include <gtk/gtk.h>
#include <gtk/gtkmain.h>

/* The debug window. */
static GtkWidget *window;

/* Create the debug window. */
bool create_debug_window(void)
{
  gtk_init();

  window = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW (window), "Suika2 Pro for Creator");
  gtk_widget_set_size_request(window, 640, 480);

  return true;
}

/* Run a main loop iteration. */
bool run_gtk_main_iteration(void)
{
  while (g_main_context_pending(NULL))
    g_main_context_iteration(NULL, FALSE);
  return true;
}

/*
 * platform.h functions
 */

/* 再開ボタンが押されたか調べる */
bool is_resume_pushed(void)
{
  return true;
}

/* 次へボタンが押されたか調べる */
bool is_next_pushed(void)
{
  return false;
}

/* 停止ボタンが押されたか調べる */
bool is_pause_pushed(void)
{
  return false;
}

/* 実行するスクリプトファイルが変更されたか調べる */
bool is_script_changed(void)
{
  return false;
}

/* 変更された実行するスクリプトファイル名を取得する */
const char *get_changed_script(void)
{
  return false;
}

/* 実行する行番号が変更されたか調べる */
bool is_line_changed(void)
{
  return false;
}

/* 変更された実行するスクリプトファイル名を取得する */
int get_changed_line(void)
{
  return false;
}

/* コマンドがアップデートされたかを調べる */
bool is_command_updated(void)
{
  return false;
}

/* アップデートされたコマンド文字列を取得する */
const char *get_updated_command(void)
{
  return "";
}

/* スクリプトがリロードされたかを調べる */
bool is_script_reloaded(void)
{
  return false;
}

/* コマンドの実行中状態を設定する */
void set_running_state(bool running, bool request_stop)
{
  UNUSED_PARAMETER(running);
  UNUSED_PARAMETER(request_stop);
}

/* デバッグ情報を更新する */
void update_debug_info(bool script_changed)
{
  UNUSED_PARAMETER(script_changed);
}
