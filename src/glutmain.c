#include "suika.h"

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

#if defined(OSX)
#include "aunit.h"
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#elif defined(LINUX)
#include "asound.h"
#include <GL/glut.h>
#endif

#include "glesrender.h"

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

/*
 * ログ1行のサイズ
 */
#define LOG_BUF_SIZE	(4096)

/*
 * ログファイル
 */
FILE *log_fp;

/*
 * 前方参照
 */

static bool init(int argc, char *argv[]);
static void cleanup(void);
static bool open_log_file(void);
static void close_log_file(void);
static void resize(int width, int height);
static void display(void);
static void mouse(int button, int state, int x, int y);

int main(int argc, char *argv[])
{
	int ret;

	/* 互換レイヤの初期化を行う */
	if (init(argc, argv)) {
		/* アプリケーション本体の初期化処理を行う */
		if (on_event_init()) {
			/* ゲームループを実行する */
			glutMainLoop();

			/* 成功 */
			ret = 0;
		} else {
			/* 失敗 */
			ret = 1;
		}

		/* アプリケーション本体の終了処理を行う */
		on_event_cleanup();
	} else {
		/* エラーメッセージを表示する */
		if (log_fp != NULL)
			printf("Check " LOG_FILE "\n");

		/* 失敗 */
		ret = 1;
	}

	/* 互換レイヤの終了処理を行う */
	cleanup();

	return ret;
}

/* 互換レイヤの初期化処理を行う */
static bool init(int argc, char *argv[])
{
#ifdef SSE_VERSIONING
	/* ベクトル命令の対応を確認する */
	x86_check_cpuid_flags();
#endif

	/* ログファイルを開く */
	if (!open_log_file())
		return false;

	/* ファイル読み書きの初期化処理を行う */
	if (!init_file())
		return false;

	/* コンフィグの初期化処理を行う */
	if (!init_conf())
		return false;

	/* オーディオの使用を開始する */
#if defined(OSX)
	if (!init_aunit()) {
		log_error("Can't initialize sound.\n");
		return false;
	}
#elif defined(LINUX)
	if (!init_asound()) {
		log_error("Can't initialize sound.\n");
		return false;
	}
#endif

	/* glutの初期化処理を行う */
	glutInit(&argc, argv);
	glutInitWindowSize(conf_window_width, conf_window_height);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow(conv_utf8_to_native(conf_window_title));
	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);

	init_opengl();

	return true;
}

/* 互換レイヤの終了処理を行う */
static void cleanup(void)
{
	/* アプリケーション本体の終了処理を行う */
	on_event_cleanup();

	/* OpenGLの使用を終了する */
	cleanup_opengl();

	/* オーディオの使用を終了する */
#if defined(OSX)
	cleanup_aunit();
#elif defined(LINUX)
	cleanup_asound();
#endif

	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* ファイル読み書きの終了処理を行う */
	cleanup_file();

	/* ログファイルを閉じる */
	close_log_file();
}

/*
 * ログ
 */

/* ログをオープンする */
static bool open_log_file(void)
{
	if (log_fp == NULL) {
		log_fp = fopen(LOG_FILE, "w");
		if (log_fp == NULL) {
			printf("Can't open log file.\n");
			return false;
		}
	}
	return true;
}

/* ログをクローズする */
static void close_log_file(void)
{
	if (log_fp != NULL)
		fclose(log_fp);
}

/*
 * GLUTのコールバック
 */

/* ウィンドウがリサイズされたとき */
static void resize(int width, int height)
{
	glutReshapeWindow(width, height);
}

/* 表示されるとき */
static void display(void)
{
	int x, y, w, h;
	bool cont;

	/* OpenGLのレンダリングを開始する */
	opengl_start_rendering();

	/* フレームの描画を行う */
	cont = on_event_frame(&x, &y, &w, &h);

	/* OpenGLの描画を終了する */
	opengl_end_rendering();

	/* 画面に反映する */
	glutSwapBuffers();

	/* 終了する場合 */
	if (!cont) {
		cleanup();
		exit(0);
	}
}

/* マウスイベント */
static void mouse(int button , int state , int x , int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		on_event_mouse_press(MOUSE_LEFT, x, y);
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		on_event_mouse_release(MOUSE_LEFT, x, y);
	} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		on_event_mouse_press(MOUSE_RIGHT, x, y);
	} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		on_event_mouse_release(MOUSE_RIGHT, x, y);
	} else if (button == 3 && state == GLUT_DOWN) {
		on_event_key_press(KEY_UP);
		on_event_key_release(KEY_UP);
	} else if (button == 4 && state == GLUT_DOWN) {
		on_event_key_press(KEY_DOWN);
		on_event_key_release(KEY_DOWN);
	} else {
		on_event_mouse_move(x, y);
	}

	glutPostRedisplay();
}

/*
 * platform.hの実装
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vsnprintf(buf, sizeof(buf), s, ap);
		fprintf(stderr, "%s", buf);
		fprintf(log_fp, "%s", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vsnprintf(buf, sizeof(buf), s, ap);
		fprintf(stderr, "%s", buf);
		fprintf(log_fp, "%s", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	va_list ap;
	char buf[LOG_BUF_SIZE];

	va_start(ap, s);
	if (log_fp != NULL) {
		vsnprintf(buf, sizeof(buf), s, ap);
		fprintf(stderr, "%s", buf);
		fprintf(log_fp, "%s", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
 *  - 変換の必要がないので引数をそのまま返す
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	assert(utf8_message != NULL);
	return utf8_message;
}

/*
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
				  pixel_t **locked_pixels, void **texture)
{
	if (!opengl_lock_texture(width, height, pixels, locked_pixels,
				 texture))
		return false;

	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture)
{
	opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
	opengl_destroy_texture(texture);
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	opengl_render_image(dst_left, dst_top, src_image, width, height,
			    src_left, src_top, alpha, bt);
}

/*
 * イメージをマスク描画でレンダリングする
 */
void render_image_mask(int dst_left, int dst_top,
                       struct image * RESTRICT src_image,
                       int width, int height, int src_left, int src_top,
                       int mask)
{
	opengl_render_image_mask(dst_left, dst_top, src_image, width,
				 height, src_left, src_top, mask);
}

/*
 * 画面をクリアする
 */
void render_clear(int left, int top, int width, int height, pixel_t color)
{
	opengl_render_clear(left, top, width, height, color);
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
