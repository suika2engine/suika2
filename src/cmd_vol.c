/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/28 作成
 *  - 2021/06/03 マスターボリュームを追加
 */

#include "suika.h"

/*
 * ボリュームコマンド
 */
bool vol_command(void)
{
	const char *stream;
	float vol, span;

	stream = get_string_param(VOL_PARAM_STREAM);
	vol = get_float_param(VOL_PARAM_VOL);
	span = get_float_param(VOL_PARAM_SPAN);

	if (vol < 0 || vol > 1.0f) {
		log_script_vol_value(vol);
		log_script_exec_footer();
		return false;
	}

	if (strcmp(stream, "bgm") == 0 ||
	    strcmp(stream, "音楽") == 0) {
		/* BGMストリーム */
		set_mixer_volume(BGM_STREAM, vol, span);
	} else if (strcmp(stream, "voice") == 0 ||
		   strcmp(stream, "声") == 0) {
		/* VOICEストリーム */
		set_mixer_volume(VOICE_STREAM, vol, span);
	} else if (strcmp(stream, "se") == 0 ||
		   strcmp(stream, "効果音") == 0) {
		/* SEストリーム */
		set_mixer_volume(SE_STREAM, vol, span);
	} else if (strcmp(stream, "BGM") == 0) {
		/* BGMグローバルボリューム */
		set_mixer_global_volume(BGM_STREAM, vol);
	} else if (strcmp(stream, "VOICE") == 0) {
		/* VOICEグローバルボリューム */
		set_mixer_global_volume(VOICE_STREAM, vol);
	} else if (strcmp(stream, "SE") == 0) {
		/* SEグローバルボリューム */
		set_mixer_global_volume(SE_STREAM, vol);
	} else {
		/* Incorrect stream name */
		log_script_mixer_stream(stream);
		log_script_exec_footer();
		return false;
	}

	return move_to_next_command();
}
