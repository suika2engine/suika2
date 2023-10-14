/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * @chooseコマンドの実装
 *  - バックエンドとして@switchの実装を用いている
 *  - @choose, @select, @switch, @newsの4つがここで実装されている
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
 *      - 2階層のユーザがいなくなったら1階層に単純化するつもり
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
 * @selectの選択肢の数
 *  - deprecatedなコマンドである@selectは、固定で3つの選択肢を持つ
 */
#define SELECT_OPTION_COUNT	(3)

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

/* @selectのラベルの引数インデックス */
#define SELECT_LABEL(n)			(SELECT_PARAM_LABEL1 + n)

/* @selectのメッセージの引数インデックス */
#define SELECT_MESSAGE(n)		(SELECT_PARAM_TEXT1 + n)

/*
 * @newsコマンドの場合の、東西南北以外の項目の開始オフセット
 *  - 先頭4つの親選択肢が北東西南に配置される
 *  - その次のアイテムを指すのがNEWS_SWITCH_BASE
 */
#define NEWS_SWITCH_BASE	(4)

/* 指定した親選択肢が無効であるか */
#define IS_PARENT_DISABLED(n)	(parent_button[n].msg == NULL)

/* システムメニューのボタンのインデックス */
#define SYSMENU_NONE	(-1)
#define SYSMENU_QSAVE	(0)
#define SYSMENU_QLOAD	(1)
#define SYSMENU_SAVE	(2)
#define SYSMENU_LOAD	(3)
#define SYSMENU_AUTO	(4)
#define SYSMENU_SKIP	(5)
#define SYSMENU_HISTORY	(6)
#define SYSMENU_CONFIG	(7)
#define SYSMENU_CUSTOM1	(8)
#define SYSMENU_CUSTOM2	(9)

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
} parent_button[PARENT_COUNT];

/* 子選択肢のボタン */
static struct child_button {
	const char *msg;
	const char *label;
	int x;
	int y;
	int w;
	int h;
} child_button[PARENT_COUNT][CHILD_COUNT];

/*
 * 選択肢の状態
 */

/* ポイントされている親項目のインデックス */
static int pointed_parent_index;

/* 選択されているされている親項目のインデックス */
static int selected_parent_index;

/* ポイントされている子項目のインデックス */
static int pointed_child_index;

/* キー操作によってポイントが変更されたか */
static bool is_selected_by_key;

/*
 * 描画の状態
 */

/* 最初の描画であるか */
static bool is_first_frame;

/* 子選択肢の最初の描画であるか */
static bool is_child_first_frame;

/* センタリングするか */
static bool is_centered;

/*
 * システムメニュー
 *  - TODO: cmd_message.cと共通化する
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
 *  - TODO: cmd_message.cと共通化する
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
static void preprocess(void);
static void main_process(int *x, int *y, int *w, int *h);
static bool postprocess(void);

/* 初期化 */
static bool init(void);
static bool get_choose_info(void);
static bool get_ichoose_info(void);
static bool get_select_info(void);
static bool get_switch_parents_info(void);
static bool get_switch_children_info(void);

/* クリック処理 */
static void process_main_click(void);
static void process_sysmenu_click(void);

/* 描画 */
static void draw_frame(int *x, int *y, int *w, int *h);
static void draw_frame_parent(int *x, int *y, int *w, int *h);
static void draw_frame_child(int *x, int *y, int *w, int *h);
static int get_pointed_parent_index(void);
static int get_pointed_child_index(void);
static void draw_fo_fi_parent(void);
static void draw_switch_parent_images(void);
static void update_switch_parent(int *x, int *y, int *w, int *h);
static void draw_fo_fi_child(void);
static void draw_switch_child_images(void);
static void update_switch_child(int *x, int *y, int *w, int *h);
static void draw_text(int x, int y, int w, int h, const char *t, bool is_news);
static void draw_keep(void);

/* システムメニュー */
static void draw_sysmenu(int *x, int *y, int *w, int *h);
static void draw_collapsed_sysmenu(int *x, int *y, int *w, int *h);
static bool is_collapsed_sysmenu_pointed(void);
static int get_sysmenu_pointed_button(void);
static void adjust_sysmenu_pointed_index(void);
static void get_sysmenu_button_rect(int btn, int *x, int *y, int *w, int *h);

/* その他 */
static void play_se(const char *file);

/* クリーンアップ */
static bool cleanup(void);

/*
 * switchコマンド
 */
bool switch_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* 前処理として、入力を受け付ける */
	preprocess();
	if (is_quick_load_failed)
		return false;

	/* メイン処理として、描画を行う */
	main_process(x, y, w, h);

	/* 後処理として、遷移を処理する */
	if (!postprocess())
		return false;

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/* 前処理として、入力を受け付ける */
static void preprocess(void)
{
	/* システムメニューが表示されていない場合 */
	if (!is_sysmenu) {
		process_main_click();
		return;
	}

	/* システムメニューが表示されている場合 */
	process_sysmenu_click();
}

/* メイン処理として、描画を行う */
static void main_process(int *x, int *y, int *w, int *h)
{
	/*
	 * クイックロードされた場合は描画を行わない
	 *  - 同じフレームで、ロード後のコマンドが実行されるため
	 */
	if (did_quick_load)
		return;

	/* 描画を行う */
	draw_frame(x, y, w, h);

	/* システムメニューの表示完了直後のフラグをクリアする */
	is_sysmenu_finished = false;
}

/* 後処理として、遷移を処理する */
static bool postprocess(void)
{
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

	/*
	 * 必要な場合はステージのサムネイルを作成する
	 *  - クイックセーブされるとき
	 *  - システムGUIに遷移するとき
	 */
	if (will_quick_save
	    ||
	    (need_save_mode || need_load_mode || need_history_mode ||
	     need_config_mode))
		draw_stage_fo_thumb();

	/* システムメニューで押されたボタンの処理を行う */
	if (will_quick_save) {
		quick_save();
		will_quick_save = false;
	} else if (need_save_mode) {
		if (!prepare_gui_mode(SAVE_GUI_FILE, false))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_load_mode) {
		if (!prepare_gui_mode(LOAD_GUI_FILE, false))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_history_mode) {
		if (!prepare_gui_mode(HISTORY_GUI_FILE, false))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	} else if (need_config_mode) {
		if (!prepare_gui_mode(CONFIG_GUI_FILE, false))
			return false;
		set_gui_options(true, false, false);
		start_gui_mode();
	}

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

	pointed_parent_index = -1;
	selected_parent_index = -1;
	pointed_child_index = -1;

	is_centered = true;

	is_first_frame = true;
	is_child_first_frame = false;

	is_sysmenu = false;
	is_sysmenu_first_frame = false;
	is_sysmenu_finished = false;

	will_quick_save = false;
	did_quick_load = false;
	need_save_mode = false;
	need_load_mode = false;
	need_history_mode = false;
	need_config_mode = false;
	is_quick_load_failed = false;

	type = get_command_type();
	if (type == COMMAND_CHOOSE) {
		/* @chooseコマンドの引数情報を取得する */
		if (!get_choose_info())
			return false;
	} else if (type == COMMAND_ICHOOSE) {
		/* @ichooseコマンドの引数情報を取得する */
		if (!get_ichoose_info())
			return false;
	} else if (type == COMMAND_SELECT) {
		/* @selectコマンドの引数情報を取得する */
		if (!get_select_info())
			return false;
	} else {
		/* 親選択肢の情報を取得する */
		if (!get_switch_parents_info())
			return false;

		/* 子選択肢の情報を取得する */
		if (!get_switch_children_info())
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

/* @chooseコマンドの引数情報を取得する */
static bool get_choose_info(void)
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
	}

	/* テキスト読み上げする */
	if (conf_tts_enable == 1) {
		speak_text(NULL);
		if (strcmp(get_system_locale(), "ja") == 0)
			speak_text("選択肢が表示されています。左右のキーを押してください。");
		else
			speak_text("Options are dispayed. Press left or right arrow key.");
	}

	return true;
}

/* @ichooseコマンドの引数情報を取得する */
static bool get_ichoose_info(void)
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
	}

	return true;
}

/* @selectコマンドの引数情報を取得する */
static bool get_select_info(void)
{
	const char *label, *msg;
	int i;

	memset(parent_button, 0, sizeof(parent_button));
	memset(child_button, 0, sizeof(child_button));

	/* 選択肢の情報を取得する */
	for (i = 0; i < SELECT_OPTION_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(SELECT_LABEL(i));

		/* メッセージを取得する */
		msg = get_string_param(SELECT_MESSAGE(i));

		/* ボタンの情報を保存する */
		parent_button[i].msg = msg;
		parent_button[i].label = label;
		parent_button[i].has_child = false;
		parent_button[i].child_count = 0;

		/* 座標を計算する */
		get_switch_rect(i, &parent_button[i].x,
				&parent_button[i].y,
				&parent_button[i].w,
				&parent_button[i].h);
	}

	return true;
}

/* 親選択肢の情報を取得する */
static bool get_switch_parents_info(void)
{
	const char *p;
	int i, parent_button_count = 0;
	bool is_first;

	memset(parent_button, 0, sizeof(parent_button));

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

		parent_button_count++;
	}
	if (parent_button_count == 0) {
		log_script_switch_no_item();
		log_script_exec_footer();
		return false;
	}

	return true;
}

/* 子選択肢の情報を取得する */
static bool get_switch_children_info(void)
{
	const char *p;
	int i, j;

	memset(child_button, 0, sizeof(child_button));

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
		}
		assert(j > 0);
		parent_button[i].child_count = j;
	}

	return true;
}

/*
 * クリック処理
 */

/* システムメニュー非表示中のクリックを処理する */
static void process_main_click(void)
{
	bool enter_sysmenu;

	/* ヒストリ画面への遷移を確認する */
	if (is_up_pressed && !conf_msgbox_history_disable && get_history_count() != 0) {
		play_se(conf_msgbox_history_se);
		need_history_mode = true;
		return;
	}

	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return;

	/* システムメニューへの遷移を確認していく */
	enter_sysmenu = false;

	/* 右クリックされたとき */
	if (selected_parent_index == -1 && is_right_button_pressed)
		enter_sysmenu = true;

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
		sysmenu_pointed_index = get_sysmenu_pointed_button();
		adjust_sysmenu_pointed_index();
		old_sysmenu_pointed_index = sysmenu_pointed_index;
		is_sysmenu_finished = false;
		return;
	}
}

/* システムメニュー表示中のクリックを処理する */
static void process_sysmenu_click(void)
{
	/* 右クリックされた場合と、エスケープキーが押下されたとき */
	if (is_right_button_pressed || is_escape_pressed) {
		/* SEを再生する */
		play_se(conf_sysmenu_leave_se);

		/* システムメニューを終了する */
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return;
	}

	/* ポイントされているシステムメニューのボタンを求める */
	old_sysmenu_pointed_index = sysmenu_pointed_index;
	sysmenu_pointed_index = get_sysmenu_pointed_button();
	adjust_sysmenu_pointed_index();

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
		if (!quick_load())
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

/*
 * 描画
 */

/* フレームを描画する */
static void draw_frame(int *x, int *y, int *w, int *h)
{
	*x = 0;
	*y = 0;
	*w = 0;
	*h = 0;

	/* セーブ画面かヒストリ画面から復帰した場合のフラグをクリアする */
	check_gui_flag();

	/* 初回描画の場合 */
	if (is_first_frame) {
		pointed_parent_index = get_pointed_parent_index();

		/* 親選択肢の描画を行う */
		draw_fo_fi_parent();
		update_switch_parent(x, y, w, h);

		/* 折りたたみシステムメニューを描画する */
		if (conf_sysmenu_transition && !is_non_interruptible())
			draw_stage_collapsed_sysmenu(false, x, y, w, h);

		/* 名前ボックス、メッセージボックスを消すため再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;

		is_first_frame = false;
		return;
	}

	/* システムメニューを表示中でない場合 */
	if (!is_sysmenu) {
		if (selected_parent_index == -1) {
			/* 親選択肢を選んでいる最中の場合 */
			draw_frame_parent(x, y, w, h);
		} else {
			/* 子選択肢を選んでいる最中の場合 */
			draw_frame_child(x, y, w, h);
		}

		/* 折りたたみシステムメニューを表示する */
		draw_collapsed_sysmenu(x, y, w, h);
	}

	/* システムメニューを表示中の場合 */
	if (is_sysmenu)
		draw_sysmenu(x, y, w, h);

	/* システムメニューを終了した直後の場合 */
	if (is_sysmenu_finished) {
		update_switch_parent(x, y, w, h);
		draw_collapsed_sysmenu(x, y, w, h);
	}
}

/* 親選択肢の描画を行う */
static void draw_frame_parent(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	new_pointed_index = get_pointed_parent_index();

	if (new_pointed_index == -1 && pointed_parent_index == -1) {
		draw_keep();
	} else if (new_pointed_index == pointed_parent_index) {
		draw_keep();
	} else {
		/* ボタンを描画する */
		pointed_parent_index = new_pointed_index;
		update_switch_parent(x, y, w, h);

		/* SEを再生する */
		if (new_pointed_index != -1 && !is_left_clicked &&
		    !is_sysmenu_finished) {
			play_se(get_command_type() == COMMAND_NEWS ?
				conf_news_change_se : conf_switch_change_se);
		}

		/* 読み上げを行う */
		if (conf_tts_enable &&
		    is_selected_by_key &&
		    parent_button[pointed_parent_index].msg != NULL) {
			speak_text(NULL);
			speak_text(parent_button[pointed_parent_index].msg);
		}
	}

	/* マウスの左ボタンでクリックされた場合 */
	if (new_pointed_index != -1 &&
	    (is_left_clicked || is_return_pressed) &&
	    !is_sysmenu_finished) {
		selected_parent_index = new_pointed_index;
		pointed_child_index = -1;
		is_child_first_frame = true;

		if (parent_button[new_pointed_index].has_child) {
			/* 子選択肢の描画を行う */
			draw_fo_fi_child();
			update_switch_child(x, y, w, h);

			/* SEを鳴らす */
			play_se(conf_switch_parent_click_se_file);
		} else {
			/* ステージをボタンなしで描画しなおす */
			draw_stage();

			/* SEを鳴らす */
			play_se(conf_switch_child_click_se_file);

			/* 繰り返し動作を終了する */
			stop_command_repetition();
		}

		/* ステージ全体を再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	}
}

/* 子選択肢の描画を行う */
static void draw_frame_child(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	if (is_right_button_pressed) {
		selected_parent_index = -1;

		/* 親選択肢の描画を行う */
		draw_fo_fi_parent();
		update_switch_parent(x, y, w, h);
		return;
	}

	new_pointed_index = get_pointed_child_index();

	if (is_child_first_frame) {
		is_child_first_frame = false;

		/* ボタンを描画する */
		pointed_child_index = new_pointed_index;
		update_switch_child(x, y, w, h);
	} else if (new_pointed_index == -1 && pointed_child_index == -1) {
		draw_keep();
	} else if (new_pointed_index == pointed_child_index) {
		draw_keep();
	} else {
		/* ボタンを描画する */
		pointed_child_index = new_pointed_index;
		update_switch_child(x, y, w, h);

		/* SEを再生する */
		if (new_pointed_index != -1 && !is_left_clicked)
			play_se(conf_switch_change_se);
	}

	/* マウスの左ボタンでクリックされた場合 */
	if (new_pointed_index != -1 && is_left_clicked) {
		/* SEを鳴らす */
		play_se(conf_switch_child_click_se_file);

		/* ステージをボタンなしで描画しなおす */
		draw_stage();
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;

		/* 繰り返し動作を終了する */
		stop_command_repetition();
	}
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
		if (pointed_parent_index == -1)
			return 0;
		if (pointed_parent_index == PARENT_COUNT - 1)
			return 0;
		if (parent_button[pointed_parent_index + 1].msg != NULL)
			return pointed_parent_index + 1;
		else
			return 0;
	}

	/* 左キーを処理する */
	if (is_left_arrow_pressed) {
		is_selected_by_key = true;
		if (pointed_parent_index == -1 ||
		    pointed_parent_index == 0) {
			for (i = PARENT_COUNT - 1; i >= 0; i--)
				if (parent_button[i].msg != NULL)
					return i;
		}
		return pointed_parent_index - 1;
	}

	/* マウスポイントを処理する */
	for (i = 0; i < PARENT_COUNT; i++) {
		if (IS_PARENT_DISABLED(i))
			continue;

		if (mouse_pos_x >= parent_button[i].x &&
		    mouse_pos_x < parent_button[i].x + parent_button[i].w &&
		    mouse_pos_y >= parent_button[i].y &&
		    mouse_pos_y < parent_button[i].y + parent_button[i].h) {
			is_selected_by_key = false;
			return i;
		}
	}

	/* キーによる選択が行われている場合は維持する */
	if (is_selected_by_key)
		return pointed_parent_index;

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

/* 親選択肢のFO/FIレイヤを描画する */
static void draw_fo_fi_parent(void)
{
	lock_layers_for_msgdraw(LAYER_FO, LAYER_FI);
	{
		draw_stage_fo_fi();
		draw_switch_parent_images();
	}
	unlock_layers_for_msgdraw(LAYER_FO, LAYER_FI);
}

/* 親選択肢のイメージを描画する */
void draw_switch_parent_images(void)
{
	int i;
	bool is_news;

	assert(selected_parent_index == -1);

	for (i = 0; i < PARENT_COUNT; i++) {
		if (IS_PARENT_DISABLED(i))
			continue;

		/* @newsの東西南北の項目であるか調べる */
		is_news = get_command_type() == COMMAND_NEWS &&
			i < NEWS_SWITCH_BASE;

		/* FO/FIレイヤにスイッチを描画する */
		if (!is_news) {
			draw_switch_fg_image(i,
					     parent_button[i].x,
					     parent_button[i].y);
			draw_switch_bg_image(i,
					     parent_button[i].x,
					     parent_button[i].y);
		} else {
			draw_news_fg_image(parent_button[i].x,
					   parent_button[i].y);
			draw_news_bg_image(parent_button[i].x,
					   parent_button[i].y);
		}

		/* テキストを描画する */
		draw_text(parent_button[i].x, parent_button[i].y,
			  parent_button[i].w, parent_button[i].h,
			  parent_button[i].msg, is_news);
	}
}

/* 親選択肢を画面に描画する */
void update_switch_parent(int *x, int *y, int *w, int *h)
{
	int i, bx, by, bw, bh;

	assert(selected_parent_index == -1);

	i = pointed_parent_index;

	/* 描画するFIレイヤの矩形を求める */
	bx = by = bw = bh = 0;
	if (i != -1) {
		bx = parent_button[i].x;
		by = parent_button[i].y;
		bw = parent_button[i].w;
		bh = parent_button[i].h;
	}

	/* FOレイヤ全体とFIレイヤの矩形を画面に描画する */
	draw_fo_all_and_fi_rect(bx, by, bw, bh);

	/* 更新範囲を設定する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* 子選択肢のFO/FIレイヤを描画する */
static void draw_fo_fi_child(void)
{
	lock_layers_for_msgdraw(LAYER_FO, LAYER_FI);
	{
		draw_stage_fo_fi();
		draw_switch_child_images();
	}
	unlock_layers_for_msgdraw(LAYER_FO, LAYER_FI);
}

/* 子選択肢のイメージを描画する */
void draw_switch_child_images(void)
{
	int i, j;

	assert(selected_parent_index != -1);
	assert(parent_button[selected_parent_index].child_count > 0);

	i = selected_parent_index;
	for (j = 0; j < parent_button[i].child_count; j++) {
		/* FO/FIレイヤにスイッチを描画する */
		draw_switch_fg_image(i,
				     child_button[i][j].x,
				     child_button[i][j].y);
		draw_switch_bg_image(i,
				     child_button[i][j].x,
				     child_button[i][j].y);

		/* テキストを描画する */
		draw_text(child_button[i][j].x, child_button[i][j].y,
			  child_button[i][j].w, child_button[i][j].h,
			  child_button[i][j].msg, false);
	}
}

/* 親選択肢を画面に描画する */
void update_switch_child(int *x, int *y, int *w, int *h)
{
	int i, j, bx, by, bw, bh;

	assert(selected_parent_index != -1);

	i = selected_parent_index;
	j = pointed_child_index;

	/* 描画するFIレイヤの矩形を求める */
	bx = by = bw = bh = 0;
	if (j != -1) {
		bx = child_button[i][j].x;
		by = child_button[i][j].y;
		bw = child_button[i][j].w;
		bh = child_button[i][j].h;
	}

	/* FO全体とFIの1矩形を描画する(GPU用) */
	draw_fo_all_and_fi_rect(bx, by, bw, bh);

	/* 更新範囲を設定する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* 選択肢のテキストを描画する */
static void draw_text(int x, int y, int w, int h, const char *text, bool is_news)
{
	struct draw_msg_context context;
	pixel_t active_color, active_outline_color;
	pixel_t inactive_color, inactive_outline_color;
	int font_size, char_count;
	int ret_x, ret_y, ret_w, ret_h;
	bool use_outline;

	/* フォントサイズを取得する */
	font_size = conf_switch_font_size > 0 ?
		conf_switch_font_size : conf_font_size;

	/* ふちどりを決定する */
	switch (conf_switch_font_outline) {
	case 0: use_outline = !conf_font_outline_remove; break;
	case 1: use_outline = true; break;
	case 2: use_outline = false; break;
	default: use_outline = false; break;
	}

	/* 色を決める FIXME: add font color for switch. */
	inactive_color =
		make_pixel_slow(0xff,
				(pixel_t)conf_font_color_r,
				(pixel_t)conf_font_color_g,
				(pixel_t)conf_font_color_b);
	inactive_outline_color =
		make_pixel_slow(0xff,
				(pixel_t)conf_font_outline_color_r,
				(pixel_t)conf_font_outline_color_g,
				(pixel_t)conf_font_outline_color_b);
	if (conf_switch_color_active) {
		active_color =
			make_pixel_slow(0xff,
					(pixel_t)conf_switch_color_active_body_r,
					(pixel_t)conf_switch_color_active_body_g,
					(pixel_t)conf_switch_color_active_body_b);
		active_outline_color =
			make_pixel_slow(0xff,
					(pixel_t)conf_switch_color_active_outline_r,
					(pixel_t)conf_switch_color_active_outline_g,
					(pixel_t)conf_switch_color_active_outline_b);
	} else {
		active_color = inactive_color;
		active_outline_color = inactive_outline_color;
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

	/* FIレイヤに選択時の色で文字を描画する */
	construct_draw_msg_context(
		&context,
		LAYER_FI,
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
		active_color,
		active_outline_color,
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
	char_count = count_chars_common(&context);
	draw_msg_common(&context, char_count, &ret_x, &ret_y, &ret_w, &ret_h);

	/* FOレイヤに非選択時の色で文字を描画する */
	construct_draw_msg_context(
		&context,
		LAYER_FO,
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
		inactive_color,
		inactive_outline_color,
		false,			/* is_dimming */
		true,			/* ignore_linefeed */
		false,			/* ignore_font*/
		false,			/* ignore_outline */
		false,			/* ignore_color */
		false,			/* ignore_size */
		false,			/* ignore_position */
		false,			/* ignore_ruby */
		true,			/* ignore_wait */
		NULL,			/* inline_wait_hook */
		conf_msgbox_tategaki);	/* use_tategaki */
	draw_msg_common(&context, char_count, &ret_x, &ret_y, &ret_w, &ret_h);
}

/* 描画する(GPU用) */
static void draw_keep(void)
{
	int x, y, w, h, i, j;

	x = y = w = h = 0;
	if (selected_parent_index == -1) {
		i = pointed_parent_index;
		if (i != -1) {
			x = parent_button[i].x;
			y = parent_button[i].y;
			w = parent_button[i].w;
			h = parent_button[i].h;
		}
	} else {
		i = selected_parent_index;
		j = pointed_child_index;
		if (j != -1) {
			x = child_button[i][j].x;
			y = child_button[i][j].y;
			w = child_button[i][j].w;
			h = child_button[i][j].h;
		}
	}

	/* FO全体とFIの1矩形を描画する(GPU用) */
	draw_fo_all_and_fi_rect_accelerated(x, y, w, h);
}

/*
 * システムメニュー
 */

/* システムメニューを描画する */
static void draw_sysmenu(int *x, int *y, int *w, int *h)
{
	int bx, by, bw, bh;
	bool qsave_sel, qload_sel, save_sel, load_sel, auto_sel, skip_sel;
	bool history_sel, config_sel, custom1_sel, custom2_sel, redraw;

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
	redraw = false;

	/* システムメニューの最初のフレームの場合、描画する */
	if (is_sysmenu_first_frame) {
		redraw = true;
		is_sysmenu_first_frame = false;
	}

	/* 選択項目に変更がある場合、描画する */
	if (sysmenu_pointed_index != old_sysmenu_pointed_index &&
	    !is_sysmenu_first_frame)
		redraw = true;

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

	/* GPUを利用している場合 */
	if (is_gpu_accelerated())
		redraw = true;

	/* 描画する */
	if (redraw) {
		/* 背景を描画する */
		get_sysmenu_rect(&bx, &by, &bw, &bh);
		union_rect(x, y, w, h, *x, *y, *w, *h, bx, by, bw, bh);
		draw_fo_rect_accelerated(bx, by, bw, bh);

		/* システムメニューを描画する */
		draw_stage_sysmenu(false,
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
				   custom2_sel,
				   x, y, w, h);
		is_sysmenu_first_frame = false;
	}
}

/* 折りたたみシステムメニューを描画する */
static void draw_collapsed_sysmenu(int *x, int *y, int *w, int *h)
{
	bool is_pointed;

 	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return;

	/* 折りたたみシステムメニューがポイントされているか調べる */
	is_pointed = is_collapsed_sysmenu_pointed();

	/* 描画する */
	draw_stage_collapsed_sysmenu(is_pointed, x, y, w, h);

	/* SEを再生する */
	if (!is_sysmenu_finished &&
	    (is_collapsed_sysmenu_pointed_prev != is_pointed))
		play_se(conf_sysmenu_collapsed_se);

	/* 折りたたみシステムメニューのポイント状態を保持する */
	is_collapsed_sysmenu_pointed_prev = is_pointed;
}

/* 折りたたみシステムメニューがポイントされているか調べる */
static bool is_collapsed_sysmenu_pointed(void)
{
	int bx, by, bw, bh;

 	/* システムメニューを常に使用しない場合 */
	if (conf_sysmenu_hidden == 2)
		return false;

	get_collapsed_sysmenu_rect(&bx, &by, &bw, &bh);
	if (mouse_pos_x >= bx && mouse_pos_x < bx + bw &&
	    mouse_pos_y >= by && mouse_pos_y < by + bh)
		return true;

	return false;
}

/* 選択中のシステムメニューのボタンを取得する */
static int get_sysmenu_pointed_button(void)
{
	int rx, ry, btn_x, btn_y, btn_w, btn_h, i;

	/* システムメニューを表示中でない場合は非選択とする */
	if (!is_sysmenu)
		return SYSMENU_NONE;

	/* マウス座標からシステムメニュー画像内座標に変換する */
	rx = mouse_pos_x - conf_sysmenu_x;
	ry = mouse_pos_y - conf_sysmenu_y;

	/* ボタンを順番に見ていく */
	for (i = SYSMENU_QSAVE; i <= SYSMENU_CONFIG; i++) {
		/* ボタンの座標を取得する */
		get_sysmenu_button_rect(i, &btn_x, &btn_y, &btn_w, &btn_h);

		/* マウスがボタンの中にあればボタンの番号を返す */
		if ((rx >= btn_x && rx < btn_x + btn_w) &&
		    (ry >= btn_y && ry < btn_y + btn_h))
			return i;
	}

	/* ボタンがポイントされていない */
	return SYSMENU_NONE;
}

/* 状態に応じて、ポイント中の項目を無効にする */
static void adjust_sysmenu_pointed_index(void)
{
	/* セーブロードが無効な場合、セーブロードのポイントを無効にする */
	if (!is_save_load_enabled() &&
	    (sysmenu_pointed_index == SYSMENU_QSAVE ||
	     sysmenu_pointed_index == SYSMENU_QLOAD ||
	     sysmenu_pointed_index == SYSMENU_SAVE ||
	     sysmenu_pointed_index == SYSMENU_LOAD))
		sysmenu_pointed_index = SYSMENU_NONE;

	/* クイックセーブデータがない場合、QLOADのポイントを無効にする */
	if (!have_quick_save_data() &&
	    sysmenu_pointed_index == SYSMENU_QLOAD)
		sysmenu_pointed_index = SYSMENU_NONE;

	/* オートとスキップのポイントを無効にする */
	if (sysmenu_pointed_index == SYSMENU_AUTO || sysmenu_pointed_index == SYSMENU_SKIP)
		sysmenu_pointed_index = SYSMENU_NONE;
}

/* システムメニューのボタンの座標を取得する */
static void get_sysmenu_button_rect(int btn, int *x, int *y, int *w, int *h)
{
	switch (btn) {
	case SYSMENU_QSAVE:
		*x = conf_sysmenu_qsave_x;
		*y = conf_sysmenu_qsave_y;
		*w = conf_sysmenu_qsave_width;
		*h = conf_sysmenu_qsave_height;
		break;
	case SYSMENU_QLOAD:
		*x = conf_sysmenu_qload_x;
		*y = conf_sysmenu_qload_y;
		*w = conf_sysmenu_qload_width;
		*h = conf_sysmenu_qload_height;
		break;
	case SYSMENU_SAVE:
		*x = conf_sysmenu_save_x;
		*y = conf_sysmenu_save_y;
		*w = conf_sysmenu_save_width;
		*h = conf_sysmenu_save_height;
		break;
	case SYSMENU_LOAD:
		*x = conf_sysmenu_load_x;
		*y = conf_sysmenu_load_y;
		*w = conf_sysmenu_load_width;
		*h = conf_sysmenu_load_height;
		break;
	case SYSMENU_AUTO:
		*x = conf_sysmenu_auto_x;
		*y = conf_sysmenu_auto_y;
		*w = conf_sysmenu_auto_width;
		*h = conf_sysmenu_auto_height;
		break;
	case SYSMENU_SKIP:
		*x = conf_sysmenu_skip_x;
		*y = conf_sysmenu_skip_y;
		*w = conf_sysmenu_skip_width;
		*h = conf_sysmenu_skip_height;
		break;
	case SYSMENU_HISTORY:
		*x = conf_sysmenu_history_x;
		*y = conf_sysmenu_history_y;
		*w = conf_sysmenu_history_width;
		*h = conf_sysmenu_history_height;
		break;
	case SYSMENU_CONFIG:
		*x = conf_sysmenu_config_x;
		*y = conf_sysmenu_config_y;
		*w = conf_sysmenu_config_width;
		*h = conf_sysmenu_config_height;
		break;
	default:
		assert(ASSERT_INVALID_BTN_INDEX);
		break;
	}
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
	int n, m;

	/* クイックロードやシステムGUIへの遷移を行う場合 */
	if (did_quick_load || need_save_mode || need_load_mode ||
	    need_history_mode || need_config_mode) {
		/* コマンドの移動を行わない */
		return true;
	}

	n = selected_parent_index;

	/*
	 * 子選択肢が選択された場合
	 *  - @switch/@newsのときだけ
	 */
	if (parent_button[n].has_child) {
		m = pointed_child_index;
		return move_to_label(child_button[n][m].label);
	}

	/*
	 * 親選択肢が選択された場合
	 *  - @choose/@selectのときは常に親選択肢
	 *  - @switch/@newsのときは子選択肢がない親選択肢のときのみ
	 */
	return move_to_label(parent_button[n].label);
}
