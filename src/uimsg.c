/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * uimsg.c: UI message management.
 */

#include "suika.h"
#include "uimsg.h"

/* False assertion */
#define INVALID_UI_MSG_ID	(0)

/*
 * Get a UI message
 */
const wchar_t *get_ui_message(int id)
{
	switch (id) {
	case UIMSG_YES:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"はい";
		case LOCALE_ZH:
			return L"是";
		default:
			return L"Yes";
		}
		break;
	case UIMSG_NO:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"いいえ";
		case LOCALE_ZH:
			return L"不是";
		default:
			return L"No";
		}
		break;
	case UIMSG_INFO:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"情報";
		case LOCALE_ZH:
			return L"信息";
		default:
			return L"Info";
		}
		break;
	case UIMSG_WARN:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"注意";
		case LOCALE_ZH:
			return L"警告";
		default:
			return L"Warn";
		}
		break;
	case UIMSG_ERROR:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"エラー";
		case LOCALE_ZH:
			return L"错误";
		default:
			return L"Error";
		}
		break;
	case UIMSG_CANNOT_OPEN_LOG:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ログファイルをオープンできません。";
		case LOCALE_ZH:
			return L"无法打开日志文件。";
		default:
			return L"Cannot open log file.";
		}
		break;
	case UIMSG_EXIT:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ゲームを終了しますか？";
		case LOCALE_ZH:
			return L"游戏结束了吗？";
		default:
			return L"Are you sure you want to quit?";
		}
		break;
	case UIMSG_TITLE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"タイトルに戻りますか？";
		case LOCALE_ZH:
			return L"回到标题？";
		default:
			return L"Are you sure you want to go to title?";
		}
		break;
	case UIMSG_DELETE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"削除してもよろしいですか？";
		case LOCALE_ZH:
			return L"删除确定要删除吗？";
		default:
			return L"Are you sure you want to delete the save data?";
		}
		break;
	case UIMSG_OVERWRITE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"上書きしてもよろしいですか？";
		case LOCALE_ZH:
			return L"您确定要覆盖吗？";
		default:
			return L"Are you sure you want to overwrite the save data?";
		}
		break;
	case UIMSG_DEFAULT:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"設定をリセットしてもよろしいですか？";
		case LOCALE_ZH:
			return L"您确定要重置设置吗？";
		default:
			return L"Are you sure you want to reset the settings?";
		}
		break;
#ifdef WIN
	case UIMSG_WIN_NO_DIRECT3D:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"Direct3Dはサポートされません。";
		case LOCALE_ZH:
			return L"不支持Direct3D。";
		default:
			return L"Direct3D is not supported.";
		}
		break;
	case UIMSG_WIN_SMALL_DISPLAY:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ディスプレイのサイズが足りません。";
		case LOCALE_ZH:
			return L"显示尺寸不足。";
		default:
			return L"Display size too small.";
		}
		break;
	case UIMSG_WIN_MENU_FILE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ファイル(&F)";
		case LOCALE_ZH:
			return L"文件(&F)";
		default:
			return L"File(&F)";
		}
		break;
	case UIMSG_WIN_MENU_VIEW:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"表示(&V)";
		case LOCALE_ZH:
			return L"展示(&V)";
		default:
			return L"View(&V)";
		}
		break;
	case UIMSG_WIN_MENU_QUIT:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"終了(&Q)";
		case LOCALE_ZH:
			return L"退出(&Q)";
		default:
			return L"Quit(&Q)";
		}
		break;
	case UIMSG_WIN_MENU_FULLSCREEN:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"フルスクリーン(&F)\tAlt+Enter";
		case LOCALE_ZH:
			return L"全屏(&F)\tAlt+Enter";
		default:
			return L"Full Screen(&F)\tAlt+Enter";
		}
		break;
#endif
	}

	/* Never come here. */
	assert(INVALID_UI_MSG_ID);
	return NULL;
}
