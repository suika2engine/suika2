/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * swicthコマンドの実装
 *  - @choose, @switch, @newsがここで実装されている
 *
 * [Changes]
 *  - 2017/08/14 作成
 *  - 2017/10/31 効果音に対応
 *  - 2019/09/17 NEWSに対応
 *  - 2022/06/17 @chooseに対応、@newsの設定項目を@switchに統合
 *  - 2023/06/26 リファクタリング
 *  - 2023/08/26 conf_msgbox_show_on_choose
 *  - 2023/09/12 メッセージ描画の共通化
 */

/*
 * [Memo]
 *  - 親選択肢と子選択肢があり、2階層のメニューになっている
 *    - 最初の頃、1階層のみで3選択肢(固定)の@selectだけがあった(cmd_select.c)
 *    - その後、@switchができて、2階層で最大8x8の選択肢になった (cmd_switch.c)
 *    - さらに、@newsができたが、これはcmd_switch.cで実装された
 *    - 最終的に、@chooseができて、1階層で最大8選択肢(可変)が実現した
 *      - これはcmd_switch.cで実装された
 *      - 理由は、同時に実装されたシステムメニューの処理を共通化するため
 *      - なので、このときcmd_select.cはなくなり、cmd_switch.cに一本化された
 *    - @switch/@newsはほぼ使われていないが、まだ維持されている
 *      - チュートリアル「ミッチーにおまかせ！」では@switchの2階層を使っている
 *        - ただ、引数が多すぎて他の方には使えないと思われる
 *      - ひとまずは2階層の実装のままにする (大して複雑でもないため)
 *    - @selectは削除された
 *    - GPU必須化に伴って、FO/FIの使用をやめ、ボタン画像を作成するように変更した
 *  - システムメニューについて
 *    - システムメニューの非表示時には、折りたたみシステムメニューが表示される
 *    - コンフィグのsysmenu.hidden=1は選択肢コマンドには影響しない
 *      - 常に折りたたみシステムメニューかシステムメニューが表示される
 *      - 理由は他にセーブ・ロードの手段がないから
 *    - sysmenu.hidden=2なら、選択肢コマンドでも表示しない
 *
 * [TODO]
 *  - システムメニュー処理はcmd_switch.cと重複するので、sysmenu.cに分離する
 */

#include "suika.h"

/* false assertion */
#define ASSERT_INVALID_BTN_INDEX (0)

/*
 * 親ボタンの最大数
 *  - @choose, @select は親ボタンのみ使用する
 */
#define PARENT_COUNT		(8)

/*
 * 親ボタン1つあたりの子ボタンの最大数
 *  - @switch, @news のときのみ使用できる
 *  - @switch, @news のときでも、引数によっては使用しない
 */
#define CHILD_COUNT		(8)

/*
 * 引数インデックスの計算
 *  - コマンドの種類によって変わる
 */

/* @chooseのラベルの引数インデックス */
#define CHOOSE_LABEL(n)			(CHOOSE_PARAM_LABEL1 + n * 2)

/* @chooseのメッセージの引数インデックス */
#define CHOOSE_MESSAGE(n)	(CHOOSE_PARAM_LABEL1 + n * 2 + 1)

/* @switchと@newsの親選択肢の引数インデックス */
#define SWITCH_PARENT_MESSAGE(n)	(SWITCH_PARAM_PARENT_M1 + n)

/* @switchと@newsの親選択肢の引数インデックス */
#define SWITCH_CHILD_LABEL(p,c)		(SWITCH_PARAM_CHILD1_L1 + 16 * p + 2 * c)

/* @switchと@newsの親選択肢の引数インデックス */
#define SWITCH_CHILD_MESSAGE(p,c)	(SWITCH_PARAM_CHILD1_M1 + 16 * p + 2 * c)

/*
 * @newsコマンドの場合の、東西南北以外の項目の開始オフセット
 *  - 先頭4つの親選択肢が北東西南に配置される
 *  - その次のアイテムを指すのがNEWS_SWITCH_BASE
 */
#define NEWS_SWITCH_BASE	(4)

/* 指定した親選択肢が無効であるか */
#define IS_PARENT_DISABLED(n)	(parent_button[n].msg == NULL)

/*
 * 選択肢の項目
 */

/* 親選択肢のボタン */
static struct parent_button {
	const char *msg;
	const char *label;
	bool has_child;
	int child_count;
	int x;
	int y;
	int w;
	int h;
	struct image *img_idle;
	struct image *img_hover;
} parent_button[PARENT_COUNT];

/* 子選択肢のボタン */
static struct child_button {
	const char *msg;
	const char *label;
	int x;
	int y;
	int w;
	int h;
	struct image *img_idle;
	struct image *img_hover;
} child_button[PARENT_COUNT][CHILD_COUNT];

/*
 * 選択肢の状態
 */

/* ポイントされている項目のインデックス */
static int pointed_index;

/* 1階層目で選択済みの親項目のインデックス */
static int selected_parent_index;

/* キー操作によってポイントが変更されたか */
static bool is_selected_by_key;

/* キー操作によってポイントが変更されたときのマウス座標 */
static int save_mouse_pos_x, save_mouse_pos_y;

/*
 * 描画の状態
 */

/* センタリングするか */
static bool is_centered;

/*
 * システムメニュー
 */

/* システムメニューを表示中か */
static bool is_sysmenu;

/* システムメニューの最初のフレームか */
static bool is_sysmenu_first_frame;

/* システムメニューが終了した直後か */
static bool is_sysmenu_finished;

/* システムメニューのどのボタンがポイントされているか */
static int sysmenu_pointed_index;

/* システムメニューのどのボタンがポイントされていたか */
static int old_sysmenu_pointed_index;

/* 折りたたみシステムメニューが前のフレームでポイントされていたか */
static bool is_collapsed_sysmenu_pointed_prev;

/*
 * システム遷移フラグ
 */

/* クイックセーブを行うか */
static bool will_quick_save;

/* クイックロードを行ったか */
static bool did_quick_load;

/* セーブモードに遷移するか */
static bool need_save_mode;

/* ロードモードに遷移するか */
static bool need_load_mode;

/* ヒストリモードに遷移するか */
static bool need_history_mode;

/* コフィグモードに遷移するか */
static bool need_config_mode;

/* クイックロードに失敗したか */
static bool is_quick_load_failed;

/*
 * 前方参照
 */

/* 主な処理 */
static void pre_process(void);
static bool blit_process(void);
static void render_process(void);
static bool post_process(void);

/* 初期化 */
static bool init(void);
static bool init_choose(void);
static bool init_ichoose(void);
static bool init_switch(void);
static void draw_text(struct image *target, const char *text, int w, int h, bool is_bg, bool is_news);

/* 入力処理 */
static void process_main_input(void);
static int get_pointed_index(void);
static int get_pointed_parent_index(void);
static int get_pointed_child_index(void);
static void process_sysmenu_input(void);
static int get_pointed_sysmenu_item_extended(void);

/* 描画 */
static void render_frame(void);
static void render_sysmenu_extended(void);
static void render_collapsed_sysmenu_extended(void);

/* その他 */
static void play_se(const char *file);

/* クリーンアップ */
static bool cleanup(void);

/*
 * switchコマンド
 */
bool switch_command(void)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	pre_process();
	if (is_quick_load_failed)
		return false;
	blit_process();
	render_process();
	if (!post_process())
		return false;

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

static void pre_process(void)
{
	/* システムメニューが表示されていない場合 */
	if (!is_sysmenu) {
		process_main_input();
		return;
	}

	/* システムメニューが表示されている場合 */
	process_sysmenu_input();
}

static bool blit_process(void)
{
	int i;

	/*
	 * 必要な場合はステージのサムネイルを作成する
	 *  - クイックセーブされるとき
	 *  - システムGUIに遷移するとき
	 */
	if (will_quick_save
	    ||
	    (need_save_mode || need_load_mode || need_history_mode ||
	     need_config_mode)) {
		draw_stage_to_thumb();
		if (selected_parent_index == -1) {
			for (i = 0; i < PARENT_COUNT; i++) {
				if (parent_button[i].img_idle == NULL)
					continue;
				draw_switch_to_thumb(parent_button[i].img_idle,
						     parent_button[i].x,
						     parent_button[i].y);
			}
		} else {
			for (i = 0; i < CHILD_COUNT; i++) {
				if (child_button[selected_parent_index][i].img_idle == NULL)
					continue;
				draw_switch_to_thumb(child_button[selected_parent_index][i].img_idle,
						     child_button[selected_parent_index][i].x,
						     child_button[selected_parent_index][i].y);
			}
		}
	}

	 if (need_save_mode) {
		if (!prepare_gui_mode(SAVE_GUI_FILE, true))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_load_mode) {
		if (!prepare_gui_mode(LOAD_GUI_FILE, true))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_history_mode) {
		if (!prepare_gui_mode(HISTORY_GUI_FILE, true))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_config_mode) {
		if (!prepare_gui_mode(CONFIG_GUI_FILE, true))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	}

	 return true;
}

static void render_process(void)
{
	/* レンダリングを行わない場合 */
	if (did_quick_load
	    ||
	    (need_save_mode || need_load_mode || need_history_mode || need_config_mode))
		return;

	/* レンダリングを行う */
	render_frame();

	/* システムメニューの表示完了直後のフラグをクリアする */
	is_sysmenu_finished = false;
}

static bool post_process(void)
{
	/* システムメニューで押されたボタンの処理を行う */
	if (will_quick_save) {
		quick_save(false);
		will_quick_save = false;
	}

	/*
	 * 必要な場合は繰り返し動作を停止する
	 *  - クイックロードされたとき
	 *  - システムGUIに遷移するとき
	 */
	if (did_quick_load
	    ||
	    (need_save_mode || need_load_mode || need_history_mode ||
	     need_config_mode))
		stop_command_repetition();

	return true;
}

/*
 * 初期化
 */

/* コマンドの初期化処理を行う */
bool init(void)
{
	int type;

	start_command_repetition();

	pointed_index = -1;
	selected_parent_index = -1;

	is_centered = true;

	is_sysmenu = false;
	is_sysmenu_first_frame = false;
	is_sysmenu_finished = false;
	is_collapsed_sysmenu_pointed_prev = false;

	will_quick_save = false;
	did_quick_load = false;
	need_save_mode = false;
	need_load_mode = false;
	need_history_mode = false;
	need_config_mode = false;
	is_quick_load_failed = false;

	/* Android NDKの場合、画像を破棄する */
#ifdef SUIKA_TARGET_ANDROID
	for (int i = 0; i < PARENT_COUNT; i++) {
		if (parent_button[i].img_idle != NULL)
			destroy_image(parent_button[i].img_idle);
		if (parent_button[i].img_hover != NULL)
			destroy_image(parent_button[i].img_hover);
		parent_button[i].img_idle = NULL;
		parent_button[i].img_hover = NULL;
		for (int j = 0; j < CHILD_COUNT; j++) {
			if (child_button[i][j].img_idle != NULL)
				destroy_image(parent_button[i].img_idle);
			if (child_button[i][j].img_hover != NULL)
				destroy_image(parent_button[i].img_hover);
		}
	}
#endif

	/* コマンドの種類ごとに初期化を行う */
	type = get_command_type();
	if (type == COMMAND_CHOOSE) {
		if (!init_choose())
			return false;
	} else if (type == COMMAND_ICHOOSE) {
		if (!init_ichoose())
			return false;
	} else {
		if (!init_switch())
			return false;
	}

	/* 名前ボックス、メッセージボックスを非表示にする */
	if (!conf_msgbox_show_on_choose) {
		show_namebox(false);
		if (type != COMMAND_ICHOOSE)
			show_msgbox(false);
		else
			show_msgbox(true);
	}
	show_click(false);

	/* オートモードを終了する */
	if (is_auto_mode()) {
		stop_auto_mode();
		show_automode_banner(false);
	}

	/* スキップモードを終了する */
	if (is_skip_mode()) {
		stop_skip_mode();
		show_skipmode_banner(false);
	}

	return true;
}

/* @chooseコマンドの初期化を行う */
static bool init_choose(void)
{
	const char *label, *msg;
	int i;

	memset(parent_button, 0, sizeof(parent_button));
	memset(child_button, 0, sizeof(child_button));

	/* 選択肢の情報を取得する */
	for (i = 0; i < PARENT_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(CHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(CHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		parent_button[i].msg = msg;
		parent_button[i].label = label;
		parent_button[i].has_child = false;
		parent_button[i].child_count = 0;

		/* 座標を計算する */
		get_switch_rect(i,
				&parent_button[i].x,
				&parent_button[i].y,
				&parent_button[i].w,
				&parent_button[i].h);

		/* idle画像を作成する */
		parent_button[i].img_idle = create_image(parent_button[i].w, parent_button[i].h);
		if (parent_button[i].img_idle == NULL)
			return false;
		draw_switch_bg_image(parent_button[i].img_idle, i);

		/* hover画像を作成する */
		parent_button[i].img_hover = create_image(parent_button[i].w, parent_button[i].h);
		if (parent_button[i].img_hover == NULL)
			return false;
		draw_switch_fg_image(parent_button[i].img_hover, i);

		/* テキストを描画する */
		draw_text(parent_button[i].img_idle, parent_button[i].msg, parent_button[i].w, parent_button[i].h, true, false);
		draw_text(parent_button[i].img_hover, parent_button[i].msg, parent_button[i].w, parent_button[i].h, false, false);
	}

	/* テキスト読み上げする */
	if (conf_tts_enable == 1) {
		speak_text(NULL);
		if (strcmp(get_system_locale(), "ja") == 0)
			speak_text("選択肢が表示されています。左右のキーを押してください。");
		else
			speak_text("Options are displayed. Press the left or right arrow key.");
	}

	return true;
}

/* @ichooseコマンドの引数情報を取得する */
static bool init_ichoose(void)
{
	const char *label, *msg;
	int i, pen_x, pen_y;

	memset(parent_button, 0, sizeof(parent_button));
	memset(child_button, 0, sizeof(child_button));

	is_centered = false;
	if (conf_msgbox_tategaki) {
		pen_x = get_pen_position_x() - conf_msgbox_margin_line;
		pen_y = conf_msgbox_y + conf_msgbox_margin_top;
	} else {
		pen_x = conf_msgbox_x + conf_msgbox_margin_left;
		pen_y = get_pen_position_y() + conf_msgbox_margin_line;
	}

	/* 選択肢の情報を取得する */
	for (i = 0; i < PARENT_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(CHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(CHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		parent_button[i].msg = msg;
		parent_button[i].label = label;
		parent_button[i].has_child = false;
		parent_button[i].child_count = 0;

		/* 座標を計算する */
		get_switch_rect(0,
				&parent_button[i].x,
				&parent_button[i].y,
				&parent_button[i].w,
				&parent_button[i].h);
		parent_button[i].x = pen_x;
		parent_button[i].y = pen_y;
		if (conf_msgbox_tategaki)
			pen_x -= conf_msgbox_margin_line;
		else
			pen_y += conf_msgbox_margin_line;

		/* idle画像を作成する */
		parent_button[i].img_idle = create_image(parent_button[i].w, parent_button[i].h);
		if (parent_button[i].img_idle == NULL)
			return false;
		draw_switch_bg_image(parent_button[i].img_idle, i);

		/* hover画像を作成する */
		parent_button[i].img_hover = create_image(parent_button[i].w, parent_button[i].h);
		if (parent_button[i].img_hover == NULL)
			return false;
		draw_switch_fg_image(parent_button[i].img_hover, i);

		/* テキストを描画する */
		draw_text(parent_button[i].img_idle, parent_button[i].msg, parent_button[i].w, parent_button[i].h, true, false);
		draw_text(parent_button[i].img_hover, parent_button[i].msg, parent_button[i].w, parent_button[i].h, false, false);
	}

	return true;
}

/* @switchの初期化を行う */
static bool init_switch(void)
{
	const char *p;
	int i, j, parent_button_count = 0;
	bool is_first;

	memset(parent_button, 0, sizeof(parent_button));
	memset(child_button, 0, sizeof(child_button));

	/* 親選択肢の情報を取得する */
	is_first = true;
	for (i = 0; i < PARENT_COUNT; i++) {
		/* 親選択肢のメッセージを取得する */
		p = get_string_param(SWITCH_PARENT_MESSAGE(i));
		assert(strcmp(p, "") != 0);

		/* @switchの場合、"*"が現れたら親選択肢の読み込みを停止する */
		if (get_command_type() == COMMAND_SWITCH) {
			if (strcmp(p, "*") == 0)
				break;
		} else {
			/* @newsの場合、"*"が現れたら選択肢をスキップする */
			if (strcmp(p, "*") == 0)
				continue;
		}

		/* メッセージを保存する */
		parent_button[i].msg = p;
		if (is_first) {
			/* 最後のメッセージとして保存する */
			if (!set_last_message(p))
				return false;
			is_first = false;
		}

		/* ラベルがなければならない */
		p = get_string_param(SWITCH_CHILD_LABEL(i, 0));
		if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
			log_script_switch_no_label();
			log_script_exec_footer();
			return false;
		}

		/* 子の最初のメッセージが"*"か省略なら、一階層のメニューと
		   判断してラベルを取得する */
		p = get_string_param(SWITCH_CHILD_MESSAGE(i, 0));
		if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
			p = get_string_param(SWITCH_CHILD_LABEL(i, 0));
			parent_button[i].label = p;
			parent_button[i].has_child = false;
			parent_button[i].child_count = 0;
		} else {
			parent_button[i].label = NULL;
			parent_button[i].has_child = true;
			parent_button[i].child_count = 0;
		}

		/* 座標を計算する */
		if (get_command_type() == COMMAND_SWITCH) {
			get_switch_rect(i,
					&parent_button[i].x,
					&parent_button[i].y,
					&parent_button[i].w,
					&parent_button[i].h);
		} else {
			get_news_rect(i,
				      &parent_button[i].x,
				      &parent_button[i].y,
				      &parent_button[i].w,
				      &parent_button[i].h);
		}

		/* idle画像を作成する */
		parent_button[i].img_idle = create_image(parent_button[i].w, parent_button[i].h);
		if (parent_button[i].img_idle == NULL)
			return false;
		draw_switch_bg_image(parent_button[i].img_idle, i);

		/* hover画像を作成する */
		parent_button[i].img_hover = create_image(parent_button[i].w, parent_button[i].h);
		if (parent_button[i].img_hover == NULL)
			return false;
		draw_switch_fg_image(parent_button[i].img_hover, i);

		/* テキストを描画する */
		draw_text(parent_button[i].img_idle,
			  parent_button[i].msg,
			  parent_button[i].w,
			  parent_button[i].h,
			  true,
			  false);
		draw_text(parent_button[i].img_hover,
			  parent_button[i].msg,
			  parent_button[i].w,
			  parent_button[i].h,
			  false,
			  false);

		parent_button_count++;
	}
	if (parent_button_count == 0) {
		log_script_switch_no_item();
		log_script_exec_footer();
		return false;
	}

	/* 子選択肢の情報を取得する */
	for (i = 0; i < PARENT_COUNT; i++) {
		/* 親選択肢が無効の場合、スキップする */
		if (IS_PARENT_DISABLED(i))
			continue;

		/* 親選択肢が子選択肢を持たない場合、スキップする */
		if (!parent_button[i].has_child)
			continue;

		/* 子選択肢の情報を取得する */
		for (j = 0; j < CHILD_COUNT; j++) {
			/* ラベルを取得し、"*"か省略が現れたらスキップする */
			p = get_string_param(SWITCH_CHILD_LABEL(i, j));
			if (strcmp(p, "*") == 0 || strcmp(p, "") == 0)
				break;
			child_button[i][j].label = p;

			/* メッセージを取得する */
			p = get_string_param(SWITCH_CHILD_MESSAGE(i, j));
			if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
				log_script_switch_no_item();
				log_script_exec_footer();
				return false;
			}
			child_button[i][j].msg = p;

			/* 座標を計算する */
			get_switch_rect(j,
					&child_button[i][j].x,
					&child_button[i][j].y,
					&child_button[i][j].w,
					&child_button[i][j].h);

			/* idle画像を作成する */
			child_button[i][j].img_idle = create_image(child_button[i][j].w, child_button[i][j].h);
			if (child_button[i][j].img_idle == NULL)
				return false;
			draw_switch_bg_image(child_button[i][j].img_idle, i);

			/* hover画像を作成する */
			child_button[i][j].img_hover = create_image(child_button[i][j].w, child_button[i][j].h);
			if (child_button[i][j].img_hover == NULL)
				return false;
			draw_switch_fg_image(child_button[i][j].img_hover, i);

			/* テキストを描画する */
			draw_text(child_button[i][j].img_idle,
				  child_button[i][j].msg,
				  child_button[i][j].w,
				  child_button[i][j].h,
				  true,
				  get_command_type() == COMMAND_NEWS);
			draw_text(child_button[i][j].img_hover,
				  child_button[i][j].msg,
				  child_button[i][j].w,
				  child_button[i][j].h,
				  false,
				  get_command_type() == COMMAND_NEWS);
		}
		assert(j > 0);
		parent_button[i].child_count = j;
	}

	return true;
}

/* 選択肢のテキストを描画する */
static void draw_text(struct image *target, const char *text, int w, int h, bool is_bg, bool is_news)
{
	struct draw_msg_context context;
	pixel_t color, outline_color;
	int font_size, char_count;
	int x, y;
	bool use_outline;

	x = 0;
	y = 0;

	/* フォントサイズを取得する */
	font_size = conf_switch_font_size > 0 ? conf_switch_font_size : conf_font_size;

	/* ふちどりを決定する */
	switch (conf_switch_font_outline) {
	case 0: use_outline = !conf_font_outline_remove; break;
	case 1: use_outline = true; break;
	case 2: use_outline = false; break;
	default: use_outline = false; break;
	}

	/* 色を決める */
	if (is_bg) {
		if (!conf_switch_color_inactive) {
			color = make_pixel(0xff,
					   (pixel_t)conf_font_color_r,
					   (pixel_t)conf_font_color_g,
					   (pixel_t)conf_font_color_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_font_outline_color_r,
						   (pixel_t)conf_font_outline_color_g,
						   (pixel_t)conf_font_outline_color_b);
		} else {
			color = make_pixel(0xff,
					   (pixel_t)conf_switch_color_inactive_body_r,
					   (pixel_t)conf_switch_color_inactive_body_g,
					   (pixel_t)conf_switch_color_inactive_body_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_switch_color_inactive_outline_r,
						   (pixel_t)conf_switch_color_inactive_outline_g,
						   (pixel_t)conf_switch_color_inactive_outline_b);
		}
	} else {
		if (!conf_switch_color_active) {
			color = make_pixel(0xff,
					   (pixel_t)conf_font_color_r,
					   (pixel_t)conf_font_color_g,
					   (pixel_t)conf_font_color_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_font_outline_color_r,
						   (pixel_t)conf_font_outline_color_g,
						   (pixel_t)conf_font_outline_color_b);
		} else {
			color = make_pixel(0xff,
					   (pixel_t)conf_switch_color_active_body_r,
					   (pixel_t)conf_switch_color_active_body_g,
					   (pixel_t)conf_switch_color_active_body_b);
			outline_color = make_pixel(0xff,
						   (pixel_t)conf_switch_color_active_outline_r,
						   (pixel_t)conf_switch_color_active_outline_g,
						   (pixel_t)conf_switch_color_active_outline_b);
		}
	}

	/* 描画位置を決める */
	if (is_centered) {
		if (!conf_msgbox_tategaki) {
			x = x + (w - get_string_width(conf_switch_font_select,
						      font_size,
						      text)) / 2;
			y += conf_switch_text_margin_y;
		} else {
			x = x + (w - font_size) / 2;
			y = y + (h - get_string_height(conf_switch_font_select,
						       font_size,
						       text)) / 2;
		}
	} else {
		y += is_news ?
			conf_news_text_margin_y :
			conf_switch_text_margin_y;
	}

	/* 文字を描画する */
	construct_draw_msg_context(
		&context,
		-1,
		text,
		conf_switch_font_select,
		font_size,
		font_size,		/* base_font_size */
		conf_font_ruby_size,	/* FIXME: namebox.ruby.sizeの導入 */
		use_outline,
		x,
		y,
		conf_window_width,
		conf_window_height,
		x,			/* left_margin */
		0,			/* right_margin */
		conf_switch_text_margin_y,
		0,			/* bottom_margin */
		0,			/* line_margin */
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,			/* is_dimming */
		true,			/* ignore_linefeed */
		false,			/* ignore_font */
		false,			/* ignore_size */
		false,			/* ignore_color */
		false,			/* ignore_size */
		false,			/* ignore_position */
		false,			/* ignore_ruby */
		true,			/* ignore_wait */
		NULL,			/* inline_wait_hook */
		conf_msgbox_tategaki);	/* use_tategaki */
	set_alternative_target_image(&context, target);
	char_count = count_chars_common(&context);
	draw_msg_common(&context, char_count);
}

/*
 * クリック処理
 */

/* システムメニュー非表示中の入力を処理する */
static void process_main_input(void)
{
	int old_pointed_index, new_pointed_index;
	bool enter_sysmenu;

	/* 選択項目の変更を処理する */
	if (selected_parent_index == -1) {
		old_pointed_index = pointed_index;
		new_pointed_index = get_pointed_index();
		if (new_pointed_index != old_pointed_index &&
		    new_pointed_index != -1 &&
		    conf_tts_enable == 1 &&
		    is_selected_by_key &&
		    parent_button[new_pointed_index].msg != NULL) {
			speak_text(NULL);
			speak_text(parent_button[pointed_index].msg);
		}
	} else {
		old_pointed_index = pointed_index;
		new_pointed_index = get_pointed_index();
		if (new_pointed_index != old_pointed_index &&
		    new_pointed_index != -1 &&
		    conf_tts_enable == 1 &&
		    is_selected_by_key &&
		    child_button[selected_parent_index][new_pointed_index].msg != NULL) {
			speak_text(NULL);
			speak_text(child_button[selected_parent_index][pointed_index].msg);
		}
	}
	if (new_pointed_index != -1 &&
	    new_pointed_index != old_pointed_index &&
	    !is_sysmenu_finished) {
		if (!is_left_clicked) {
			play_se(get_command_type() == COMMAND_NEWS ? conf_news_change_se : conf_switch_change_se);
		} else {
			play_se(conf_switch_parent_click_se_file);
		}
	}
	pointed_index = new_pointed_index;

	/* ヒストリ画面への遷移を確認する */
	if (is_up_pressed &&
	    !conf_msgbox_history_disable &&
	    get_history_count() != 0) {
		play_se(conf_msgbox_history_se);
		need_history_mode = true;
		return;
	}

	/* キー操作を受け付ける */
	if (is_s_pressed && conf_sysmenu_hidden != 2) {
		play_se(conf_sysmenu_save_se);
		need_save_mode = true;
		return;
	} else if (is_l_pressed && conf_sysmenu_hidden != 2) {
		play_se(conf_sysmenu_load_se);
		need_load_mode = true;
		return;
	} else if (is_h_pressed &&
		   !conf_msgbox_history_disable &&
		   get_history_count() != 0) {
		play_se(conf_msgbox_history_se);
		need_history_mode = true;
		return;
	}

	/* マウスの左ボタンでクリックされた場合 */
	if (pointed_index != -1 &&
	    (is_left_clicked || is_return_pressed) &&
	    !is_sysmenu_finished) {
		if (selected_parent_index == -1) {
			play_se(conf_switch_parent_click_se_file);
			if (!parent_button[pointed_index].has_child)
				stop_command_repetition();
			else
				selected_parent_index = pointed_index;
		} else {
			play_se(conf_switch_child_click_se_file);
			stop_command_repetition();
		}
	}

	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return;

	/* システムメニューへの遷移を確認していく */
	enter_sysmenu = false;

	/* 右クリックされたとき */
	if (is_right_clicked) {
		if (selected_parent_index == -1)
			enter_sysmenu = true;
		else
			selected_parent_index = -1;
	}

	/* エスケープキーが押下されたとき */
	if (is_escape_pressed)
		enter_sysmenu = true;

	/* 折りたたみシステムメニューがクリックされたとき */
	if (is_left_clicked && is_collapsed_sysmenu_pointed())
		enter_sysmenu = true;

	/* システムメニューを開始するとき */
	if (enter_sysmenu) {
		/* SEを再生する */
		play_se(conf_sysmenu_enter_se);

		/* システムメニューを表示する */
		is_sysmenu = true;
		is_sysmenu_first_frame = true;
		sysmenu_pointed_index = get_pointed_sysmenu_item_extended();
		old_sysmenu_pointed_index = sysmenu_pointed_index;
		is_sysmenu_finished = false;
		return;
	}
}

/* ポイントされている選択肢を取得する */
static int get_pointed_index(void)
{
	if (selected_parent_index == -1)
		return get_pointed_parent_index();
	else
		return get_pointed_child_index();
}

/* 親選択肢でポイントされているものを取得する */
static int get_pointed_parent_index(void)
{
	int i;

	/* システムメニュー表示中は選択しない */
	if (is_sysmenu)
		return -1;

	/* 右キーを処理する */
	if (is_right_arrow_pressed) {
		is_selected_by_key = true;
		save_mouse_pos_x = mouse_pos_x;
		save_mouse_pos_y = mouse_pos_y;
		if (pointed_index == -1)
			return 0;
		if (pointed_index == PARENT_COUNT - 1)
			return 0;
		if (parent_button[pointed_index + 1].msg != NULL)
			return pointed_index + 1;
		else
			return 0;
	}

	/* 左キーを処理する */
	if (is_left_arrow_pressed) {
		is_selected_by_key = true;
		save_mouse_pos_x = mouse_pos_x;
		save_mouse_pos_y = mouse_pos_y;
		if (pointed_index == -1 ||
		    pointed_index == 0) {
			for (i = PARENT_COUNT - 1; i >= 0; i--)
				if (parent_button[i].msg != NULL)
					return i;
		}
		return pointed_index - 1;
	}

	/* マウスポイントを処理する */
	for (i = 0; i < PARENT_COUNT; i++) {
		if (IS_PARENT_DISABLED(i))
			continue;

		if (mouse_pos_x >= parent_button[i].x &&
		    mouse_pos_x < parent_button[i].x + parent_button[i].w &&
		    mouse_pos_y >= parent_button[i].y &&
		    mouse_pos_y < parent_button[i].y + parent_button[i].h) {
			/* キーで選択済みの項目があり、マウスが移動していない場合 */
			if (is_selected_by_key &&
			    mouse_pos_x == save_mouse_pos_x &&
			    mouse_pos_y == save_mouse_pos_y)
				continue;
			is_selected_by_key = false;
			return i;
		}
	}

	/* キーによる選択が行われている場合は維持する */
	if (is_selected_by_key)
		return pointed_index;

	/* その他の場合、何も選択しない */
	return -1;
}

/* 子選択肢でポイントされているものを取得する */
static int get_pointed_child_index(void)
{
	int i, n;

	/* システムメニュー表示中は選択しない */
	if (is_sysmenu)
		return -1;

	n = selected_parent_index;
	for (i = 0; i < parent_button[n].child_count; i++) {
		if (mouse_pos_x >= child_button[n][i].x &&
		    mouse_pos_x < child_button[n][i].x +
		    child_button[n][i].w &&
		    mouse_pos_y >= child_button[n][i].y &&
		    mouse_pos_y < child_button[n][i].y +
		    child_button[n][i].h)
			return i;
	}

	return -1;
}

/* システムメニュー表示中のクリックを処理する */
static void process_sysmenu_input(void)
{
	/* キー操作を受け付ける */
	if (is_s_pressed) {
		play_se(conf_sysmenu_save_se);
		need_save_mode = true;
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	} else if (is_l_pressed) {
		play_se(conf_sysmenu_load_se);
		need_load_mode = true;
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	} else if (is_h_pressed) {
		play_se(conf_sysmenu_history_se);
		need_history_mode = true;
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* 右クリックされた場合と、エスケープキーが押下されたとき */
	if (is_right_clicked || is_escape_pressed) {
		/* SEを再生する */
		play_se(conf_sysmenu_leave_se);

		/* システムメニューを終了する */
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* ポイントされているシステムメニューのボタンを求める */
	old_sysmenu_pointed_index = sysmenu_pointed_index;
	sysmenu_pointed_index = get_pointed_sysmenu_item_extended();

	/* ポイントされている項目が変化した場合で、クリックではない場合 */
	if (sysmenu_pointed_index != old_sysmenu_pointed_index &&
	    sysmenu_pointed_index != SYSMENU_NONE &&
	    !is_left_clicked) {
		/* ただし最初のフレームですでにポイントされていた場合は除く */
		if (!is_sysmenu_first_frame) {
			/* SEを再生する */		     
			play_se(conf_sysmenu_change_se);
		}
	}

	/* ボタンのないところを左クリックされた場合 */
	if (sysmenu_pointed_index == SYSMENU_NONE && is_left_clicked) {
		/* SEを再生する */
		play_se(conf_sysmenu_leave_se);

		/* システムメニューを終了する */
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* 左クリックされていない場合、何もしない */
	if (!is_left_clicked)
		return;

	/* ボタンを処理する */
	switch (sysmenu_pointed_index) {
	case SYSMENU_QSAVE:
		play_se(conf_sysmenu_qsave_se);
		will_quick_save = true;
		break;
	case SYSMENU_QLOAD:
		play_se(conf_sysmenu_qload_se);
		if (!quick_load(false))
			is_quick_load_failed = true;
		did_quick_load = true;
		break;
	case SYSMENU_SAVE:
		play_se(conf_sysmenu_save_se);
		need_save_mode = true;
		break;
	case SYSMENU_LOAD:
		play_se(conf_sysmenu_load_se);
		need_load_mode = true;
		break;
	case SYSMENU_HISTORY:
		/* ヒストリがある場合のみ */
		if (get_history_count() > 0) {
			play_se(conf_sysmenu_history_se);
			need_history_mode = true;
		}
		break;
	case SYSMENU_CONFIG:
		play_se(conf_sysmenu_config_se);
		need_config_mode = true;
		break;
	default:
		assert(0);
		break;
	}

	/* システムメニューを終了する */
	is_sysmenu = false;
	is_sysmenu_finished = true;
}

/* 選択中のシステムメニューのボタンを取得する */
static int get_pointed_sysmenu_item_extended(void)
{
	int index;

	/* システムメニューを表示中でない場合は非選択とする */
	if (!is_sysmenu)
		return SYSMENU_NONE;

	/* ポイントされているボタンを返す */
	index = get_pointed_sysmenu_item();

	/* セーブロードが無効な場合、セーブロードのポイントを無効にする */
	if (!is_save_load_enabled() &&
	    (index == SYSMENU_QSAVE ||
	     index == SYSMENU_QLOAD ||
	     index == SYSMENU_SAVE ||
	     index == SYSMENU_LOAD))
		index = SYSMENU_NONE;

	/* クイックセーブデータがない場合、QLOADのポイントを無効にする */
	if (!have_quick_save_data() &&
	    index == SYSMENU_QLOAD)
		index = SYSMENU_NONE;

	/* オートとスキップのポイントを無効にする */
	if (index == SYSMENU_AUTO || index == SYSMENU_SKIP)
		index  = SYSMENU_NONE;

	return index;
}

/*
 * 描画
 */

/* フレームを描画する */
static void render_frame(void)
{
	int i;

	/* セーブ画面かヒストリ画面から復帰した場合のフラグをクリアする */
	check_gui_flag();

	/* ステージを描画する */
	render_stage();

	/* 選択肢を描画する */
	if (selected_parent_index == -1) {
		for (i = 0; i < PARENT_COUNT; i++) {
			struct image *img = i != pointed_index && i != -1?
				parent_button[i].img_idle : parent_button[i].img_hover;
			if (img == NULL)
				break;
			render_image_normal(parent_button[i].x,
					    parent_button[i].y,
					    img->width,
					    img->height,
					    img,
					    0,
					    0,
					    img->width,
					    img->height,
					    255);
		}
	} else {
		for (i = 0; i < CHILD_COUNT; i++) {
			struct image *img = i != pointed_index && i != -1 ?
				child_button[selected_parent_index][i].img_idle :
				child_button[selected_parent_index][i].img_hover;
			if (img == NULL)
				break;
			render_image_normal(child_button[selected_parent_index][i].x,
					    child_button[selected_parent_index][i].y,
					    img->width,
					    img->height,
					    img,
					    0,
					    0,
					    img->width,
					    img->height,
					    255);
		}
	}

	/* 折りたたみシステムメニューを描画する */
	if (!is_non_interruptible() && !is_sysmenu)
		render_collapsed_sysmenu_extended();

	/* システムメニューを表示する */
	if (!is_non_interruptible() && is_sysmenu)
		render_sysmenu_extended();
}

/*
 * システムメニュー
 */

/* システムメニューを描画する */
static void render_sysmenu_extended(void)
{
	bool qsave_sel, qload_sel, save_sel, load_sel, auto_sel, skip_sel;
	bool history_sel, config_sel, custom1_sel, custom2_sel;

	/* 描画するかの判定状態を初期化する */
	qsave_sel = false;
	qload_sel = false;
	save_sel = false;
	load_sel = false;
	auto_sel = false;
	skip_sel = false;
	history_sel = false;
	config_sel = false;
	custom1_sel = false;
	custom2_sel = false;

	/* クイックセーブボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_QSAVE)
		qsave_sel = true;

	/* クイックロードボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_QLOAD)
		qload_sel = true;

	/* セーブボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_SAVE)
		save_sel = true;

	/* ロードボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_LOAD)
		load_sel = true;

	/* ヒストリがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_HISTORY)
		history_sel = true;

	/* コンフィグがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CONFIG)
		config_sel = true;

	/* CUSTOM1がポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CUSTOM1)
		custom1_sel = true;

	/* CUSTOM2がポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CUSTOM2)
		custom2_sel = true;

	/* システムメニューを描画する */
	render_sysmenu(false,
		       false,
		       is_save_load_enabled(),
		       is_save_load_enabled() &&
		       have_quick_save_data(),
		       qsave_sel,
		       qload_sel,
		       save_sel,
		       load_sel,
		       auto_sel,
		       skip_sel,
		       history_sel,
		       config_sel,
		       custom1_sel,
		       custom2_sel);
	is_sysmenu_first_frame = false;
}

/* 折りたたみシステムメニューを描画する */
static void render_collapsed_sysmenu_extended(void)
{
	bool is_pointed;

 	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return;

	/* 折りたたみシステムメニューがポイントされているか調べる */
	is_pointed = is_collapsed_sysmenu_pointed();

	/* 描画する */
	render_collapsed_sysmenu(is_pointed);

	/* SEを再生する */
	if (!is_sysmenu_finished &&
	    (is_collapsed_sysmenu_pointed_prev != is_pointed))
		play_se(conf_sysmenu_collapsed_se);

	/* 折りたたみシステムメニューのポイント状態を保持する */
	is_collapsed_sysmenu_pointed_prev = is_pointed;
}

/*
 * その他
 */

/* SEを再生する */
static void play_se(const char *file)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(SYS_STREAM, w);
}

/*
 * クリーンアップ
 */

/* コマンドを終了する */
static bool cleanup(void)
{
	int i, j;

	/* クイックロードやシステムGUIへの遷移を行う場合 */
	if (did_quick_load || need_save_mode || need_load_mode ||
	    need_history_mode || need_config_mode) {
		/* コマンドの移動を行わない */
		return true;
	}

	/* 画像を破棄する */
	for (i = 0; i < PARENT_COUNT; i++) {
		if (parent_button[i].img_idle != NULL)
			destroy_image(parent_button[i].img_idle);
		if (parent_button[i].img_hover != NULL)
			destroy_image(parent_button[i].img_hover);
		parent_button[i].img_idle = NULL;
		parent_button[i].img_hover = NULL;

		if (!parent_button[i].has_child)
			continue;

		for (j = 0; j < CHILD_COUNT; j++) {
			if (child_button[i][j].img_idle != NULL)
				destroy_image(child_button[i][j].img_idle);
			if (child_button[i][j].img_hover != NULL)
				destroy_image(child_button[i][j].img_hover);
		}
	}

	/*
	 * 子選択肢が選択された場合
	 *  - @switch/@newsのときだけ
	 */
	if (selected_parent_index != -1 && parent_button[selected_parent_index].has_child)
		return move_to_label(child_button[selected_parent_index][pointed_index].label);

	/*
	 * 親選択肢が選択された場合
	 *  - @choose/@selectのときは常に親選択肢
	 *  - @switch/@newsのときは子選択肢がない親選択肢のときのみ
	 */
	return move_to_label(parent_button[pointed_index].label);
}
