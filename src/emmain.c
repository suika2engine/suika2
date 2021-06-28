#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

#include "suika.h"
#include "emopenal.h"

/*
 * 背景イメージ
 */
struct image *back_image;

/*
 * 前方参照
 */
static bool create_back_image(void);
static EM_BOOL loop_iter(double time, void * userData);
static EM_BOOL cb_mousemove(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData);
static EM_BOOL cb_mousedown(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData);
static EM_BOOL cb_mouseup(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData);

int main(void)
{
	/* パッケージの初期化処理を行う */
	if(!init_file())
		return 1;

	/* コンフィグの初期化処理を行う */
	if(!init_conf())
		return 1;
	
	/* サウンドの初期化処理を行う */
	if (!init_openal())
		return 1;

	/* バックイメージを作成する */
	if (!create_back_image())
		return 1;
	
	/* 初期化イベントを処理する */
	if(!on_event_init())
		return 1;

	/* イベントの登録をする */
	emscripten_set_mousedown_callback("canvas", 0, true, cb_mousedown);
	emscripten_set_mouseup_callback("canvas", 0, true, cb_mouseup);
	emscripten_set_mousemove_callback("canvas", 0, true, cb_mousemove);

	/* アニメーションの処理を開始する */
	emscripten_request_animation_frame_loop(loop_iter, 0);

	return 0;
}

/* 背景イメージを作成する */
static bool create_back_image(void)
{
	/* キャンバスサイズを設定する */
	emscripten_set_canvas_element_size("canvas", conf_window_width, conf_window_height);
	EM_ASM_({resizeWindow(null);});

	/* 背景イメージを作成する */
	back_image = create_image(conf_window_width, conf_window_height);
	if (back_image == NULL)
		return false;

	/* 初期状態でバックイメージを白く塗り潰す */
	if (conf_window_white)
		clear_image_white(back_image);

	return true;
}

/* フレームを処理する */
static EM_BOOL loop_iter(double time, void * userData)
{
	int x, y, w, h;
	static bool stop = false;

	/* 停止済みであれば */
	if (stop)
		return EM_FALSE;

	/* サウンドの処理を行う */
	fill_sound_buffer();

	/* フレームイベントを呼び出す */
	x = y = w = h = 0;
	if (!on_event_frame(&x, &y, &w, &h)) {
		stop = true;
		EM_FALSE;	/* スクリプトの終端に達した */
	}

	/* 描画を行わない場合 */
	if (w == 0 || h == 0)
		return EM_TRUE;

	/* 描画を行う */
	pixel_t *p;
	p = get_image_pixels(back_image);
	EM_ASM_({
			let data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
			let context = Module['canvas'].getContext('2d');
			let imageData = context.getImageData(0, 0, $1, $2);
			imageData.data.set(data);
			context.putImageData(imageData, 0, 0);
		}, p, conf_window_width, conf_window_height);

	return EM_TRUE;
}

/* mousemoveのコールバック */
static EM_BOOL cb_mousemove(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y;

	printf("(%ld,%ld)\n", mouseEvent->targetX, mouseEvent->targetY);

	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);
	on_event_mouse_move(x, y);
	return EM_TRUE;
}

/* Mousedownのコールバック */
static EM_BOOL cb_mousedown(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y, button;

	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	if (mouseEvent->button == 0)
		button = MOUSE_LEFT;
	else
		button = MOUSE_RIGHT;

	on_event_mouse_press(button, x, y);
	return EM_TRUE;
}

/* mouseupのコールバック */
static EM_BOOL cb_mouseup(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y, button;

	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	if (mouseEvent->button == 0)
		button = MOUSE_LEFT;
	else
		button = MOUSE_RIGHT;

	on_event_mouse_release(button, x, y);
	return EM_TRUE;
}

/*
 * platform.hの実装
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	return true;
}

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	return utf8_message;
}

/*
 * セーブディレクトリを作成する
 */
bool make_sav_dir(void)
{
	struct stat st = {0};

	if (stat(SAVE_DIR, &st) == -1)
		mkdir(SAVE_DIR, 0700);

	return true;
}

/*
 * データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
 */
char *make_valid_path(const char *dir, const char *fname)
{
	char *buf;
	size_t len;

	if (dir == NULL)
		dir = "";

	/* パスのメモリを確保する */
	len = strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(len);
	if (buf == NULL) {
		log_memory();
		return NULL;
	}

	strcpy(buf, dir);
	if (strlen(dir) != 0)
		strcat(buf, "/");
	strcat(buf, fname);

	return buf;
}

/*
 * バックイメージを取得する
 */
struct image *get_back_image(void)
{
	return back_image;
}

/*
 * タイマをリセットする
 */
void reset_stop_watch(stop_watch_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
int get_stop_watch_lap(stop_watch_t *t)
{
	struct timeval tv;
	stop_watch_t end;
	
	gettimeofday(&tv, NULL);

	end = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	if (end < *t) {
		/* オーバーフローの場合、タイマをリセットして0を返す */
		reset_stop_watch(t);
		return 0;
	}

	return (int)(end - *t);
}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	/* stub */
	return true;
}

/*
 * タイトルに戻るダイアログを表示する
 */
bool title_dialog(void)
{
	/* stub */
	return true;
}
