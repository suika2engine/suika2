#include "mainwindow.h"
#include "./ui_mainwindow.h"

extern "C" {
#include "suika.h"
#include "glrender.h"
};

#include <QMessageBox>
#include <QDir>
#include <QLocale>

#include <chrono>

MainWindow *MainWindow::obj;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Save the unique instance.
    assert(MainWindow::obj == nullptr);
    MainWindow::obj = this;

    // Setup a 33ms timer for OpenGLWidget redrawing.
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->start(33);

    // Clear the status flags.
    m_isRunning = false;
    m_isResumePressed = false;
    m_isNextPressed = false;
    m_isPausePressed = false;
    m_isChangeScriptPressed = false;
    m_isChangeLinePressed = false;
    m_isUpdatePressed = false;
    m_isReloadPressed = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Set a view state for when we are waiting for a command finish by a pause.
void MainWindow::setWaitingState()
{
/*
    // ウィンドウのタイトルを設定する
    [debugWindowController setTitle:isEnglish ?
                           @"Waiting for command finish..." :
                           @"コマンドの完了を待機中..."];

    // 続けるボタンを無効にする
    [debugWindowController setResumeButton:NO text:isEnglish ?
                           @"Resume" :
                           @"続ける"];

    // 次へボタンを無効にする
    [debugWindowController setNextButton:NO text:isEnglish ?
                           @"Next" :
                           @"次へ"];

    // 停止ボタンを無効にする
    [debugWindowController setPauseButton:NO text:isEnglish ?
                           @"Pause" :
                           @"停止"];

    // スクリプトテキストボックスを無効にする
    [debugWindowController enableScriptTextField:NO];

    // スクリプト変更ボタンを無効にする
    [debugWindowController enableScriptUpdateButton:NO];

    // スクリプト選択ボタンを無効にする
    //[debugWindowController enableScriptOpenButton:NO];

    // 行番号ラベルを設定する
    [debugWindowController setLineNumberLabel:isEnglish ?
                           @"Current Waiting Line:" :
                           @"現在完了待ちの行番号:"];

    // 行番号テキストボックスを無効にする
    [debugWindowController enableLineNumberTextField:NO];

    // 行番号変更ボタンを無効にする
    [debugWindowController enableLineNumberUpdateButton:NO];

    // コマンドラベルを設定する
    [debugWindowController setCommandLabel:isEnglish ?
                           @"Current Waiting Command:" :
                           @"現在完了待ちのコマンド:"];

    // コマンドテキストボックスを無効にする
    [debugWindowController enableCommandTextField:NO];

    // コマンドアップデートボタンを無効にする
    [debugWindowController enableCommandUpdateButton:NO];

    // コマンドリセットボタンを無効にする
    //[debugWindowController enableCommandResetButton:NO];

    // リストボックスを有効にする
    [debugWindowController enableScriptTableView:NO];

    // エラーを探すを無効にする
    [debugWindowController enableNextErrorButton:NO];

    // 上書き保存ボタンを無効にする
    [debugWindowController enableOverwriteButton:NO];

    // 再読み込みボタンを無効にする
    [debugWindowController enableReloadButton:NO];

    // 変数のテキストボックスを無効にする
    [debugWindowController enableVariableTextField:NO];

    // 変数の書き込みボタンを無効にする
    [debugWindowController enableVariableUpdateButton:NO];

    // スクリプトを開くメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:100] setEnabled:NO];

    // 上書き保存メニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:101] setEnabled:NO];

    // パッケージエクスポートメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:107] setEnabled:NO];

    // 続けるメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:102] setEnabled:NO];

    // 次へメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:103] setEnabled:NO];

    // 停止メニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:104] setEnabled:NO];

    // 次のエラー箇所へメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:105] setEnabled:NO];

    // 再読み込みメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:106] setEnabled:NO];
*/
}

// Set a view state for when we are running a command.
void MainWindow::setRunningState(void)
{
/*
    // ウィンドウのタイトルを設定する
    [debugWindowController setTitle:isEnglish ?
                           @"Running..." :
                           @"実行中..."];

    // 続けるボタンを無効にする
    [debugWindowController setResumeButton:NO text:isEnglish ?
                           @"Resume" :
                           @"続ける"];

    // 次へボタンを無効にする
    [debugWindowController setNextButton:NO text:isEnglish ?
                           @"Next" :
                           @"次へ"];

    // 停止ボタンを有効にする
    [debugWindowController setPauseButton:TRUE text:isEnglish ?
                           @"Pause" :
                           @"停止"];

    // スクリプトテキストボックスを無効にする
    [debugWindowController enableScriptTextField:NO];

    // スクリプト変更ボタンを無効にする
    [debugWindowController enableScriptUpdateButton:NO];

    // スクリプト選択ボタンを無効にする
    //[debugWindowController enableScriptOpenButton:NO];

    // 行番号ラベルを設定する
    [debugWindowController setLineNumberLabel:isEnglish ?
                           @"Current Running Line:" :
                           @"現在実行中の行番号:"];

    // 行番号テキストボックスを無効にする
    [debugWindowController enableLineNumberTextField:NO];

    // 行番号変更ボタンを無効にする
    [debugWindowController enableLineNumberUpdateButton:NO];

    // コマンドラベルを設定する
    [debugWindowController setCommandLabel:isEnglish ?
                           @"Current Running Command:" :
                           @"現在実行中のコマンド:"];

    // コマンドテキストボックスを無効にする
    [debugWindowController enableCommandTextField:NO];

    // コマンドアップデートボタンを無効にする
    [debugWindowController enableCommandUpdateButton:NO];

    // コマンドリセットボタンを無効にする
    //[debugWindowController enableCommandResetButton:NO];

    // リストボックスを有効にする
    [debugWindowController enableScriptTableView:NO];

    // エラーを探すを無効にする
    [debugWindowController enableNextErrorButton:NO];

    // 上書きボタンを無効にする
    [debugWindowController enableOverwriteButton:NO];

    // 再読み込みボタンを無効にする
    [debugWindowController enableReloadButton:NO];

    // 変数のテキストボックスを無効にする
    [debugWindowController enableVariableTextField:NO];

    // 変数の書き込みボタンを無効にする
    [debugWindowController enableVariableUpdateButton:NO];

    // スクリプトを開くメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:100] setEnabled:NO];

    // 上書き保存メニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:101] setEnabled:NO];

    // パッケージエクスポートメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:107] setEnabled:NO];

    // 続けるメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:102] setEnabled:NO];

    // 次へメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:103] setEnabled:NO];

    // 停止メニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:104] setEnabled:YES];

    // 次のエラー箇所へメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:105] setEnabled:NO];

    // 再読み込みメニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:106] setEnabled:NO];
*/
}

// Set a view state for when we are completely pausing.
void MainWindow::setStoppedState(void)
{
/*
    // ウィンドウのタイトルを設定する
    [debugWindowController setTitle:isEnglish ?
                           @"Stopped" :
                           @"停止中"];

    // 続けるボタンを有効にする
    [debugWindowController setResumeButton:YES text:isEnglish ?
                           @"Resume" :
                           @"続ける"];

    // 次へボタンを有効にする
    [debugWindowController setNextButton:YES text:isEnglish ?
                           @"Next" :
                           @"次へ"];

    // 停止ボタンを無効にする
    [debugWindowController setPauseButton:NO text:isEnglish ?
                           @"Pause" :
                           @"停止"];

    // スクリプトテキストボックスを有効にする
    [debugWindowController enableScriptTextField:YES];

    // スクリプト変更ボタンを有効にする
    [debugWindowController enableScriptUpdateButton:YES];

    // スクリプト選択ボタンを有効にする
    //[debugWindowController enableScriptOpenButton:YES];

    // 行番号ラベルを設定する
    [debugWindowController setLineNumberLabel:isEnglish ?
                           @"Next Line to be Executed:" :
                           @"次に実行される行番号:"];

    // 行番号テキストボックスを有効にする
    [debugWindowController enableLineNumberTextField:YES];

    // 行番号変更ボタンを有効にする
    [debugWindowController enableLineNumberUpdateButton:YES];

    // コマンドラベルを設定する
    [debugWindowController setCommandLabel:isEnglish ?
                           @"Next Command to be Executed:" :
                           @"次に実行されるコマンド:"];

    // コマンドテキストボックスを有効にする
    [debugWindowController enableCommandTextField:YES];

    // コマンドアップデートボタンを有効にする
    [debugWindowController enableCommandUpdateButton:YES];

    // コマンドリセットボタンを有効にする
    //[debugWindowController enableCommandResetButton:YES];

    // リストボックスを有効にする
    [debugWindowController enableScriptTableView:YES];

    // エラーを探すを有効にする
    [debugWindowController enableNextErrorButton:YES];

    // 上書き保存ボタンを有効にする
    [debugWindowController enableOverwriteButton:YES];

    // 再読み込みボタンを有効にする
    [debugWindowController enableReloadButton:YES];

    // 変数のテキストボックスを有効にする
    [debugWindowController enableVariableTextField:YES];

    // 変数の書き込みボタンを有効にする
    [debugWindowController enableVariableUpdateButton:YES];

    // スクリプトを開くメニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:100] setEnabled:YES];

    // 上書き保存メニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:101] setEnabled:YES];

    // パッケージエクスポートメニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:1] submenu] itemWithTag:107] setEnabled:YES];

    // 続けるメニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:102] setEnabled:YES];

    // 次へメニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:103] setEnabled:YES];

    // 停止メニューを無効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:104] setEnabled:NO];

    // 次のエラー箇所へメニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:105] setEnabled:YES];

    // 再読み込みメニューを有効にする
    [[[[[NSApp mainMenu]
           itemAtIndex:2] submenu] itemWithTag:106] setEnabled:YES];
*/
}

// A timer callback for OpenGL redrawing.
void MainWindow::onTimer()
{
    ui->openGLWidget->update();
}

void MainWindow::on_continutButton_clicked()
{
    m_isResumePressed = true;
}

void MainWindow::on_nextButton_clicked()
{
    m_isNextPressed = true;
}

void MainWindow::on_stopButton_clicked()
{
}

void MainWindow::on_fileNameEdit_returnPressed()
{
    //m_isChangeScriptPressed = true;
}

void MainWindow::on_lineNumberEdit_returnPressed()
{
    //m_isChangeScriptLinePressed = true;
}

/*
 * A HAL (platform.h API) implementation for Qt.
 */

extern "C" {

//
// Put an INFO level log.
//
bool log_info(const char *s, ...)
{
    char buf[1024];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(buf, sizeof(buf), s, ap);
    va_end(ap);

    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Information);
    msgbox.setWindowTitle("Info");
    msgbox.setText(buf);
    msgbox.addButton(QMessageBox::Close);
    msgbox.exec();

    return true;
}

//
// Put a WARN level log.
//
bool log_warn(const char *s, ...)
{
    char buf[1024];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(buf, sizeof(buf), s, ap);
    va_end(ap);

    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.setWindowTitle("Warn");
    msgbox.setText(buf);
    msgbox.addButton(QMessageBox::Close);
    msgbox.exec();

    return true;
}

//
// Put an ERROR level log.
//
bool log_error(const char *s, ...)
{
    char buf[1024];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(buf, sizeof(buf), s, ap);
    va_end(ap);

    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Warning); // Intended, it's not critical actually.
    msgbox.setWindowTitle("Error");
    msgbox.setText(buf);
    msgbox.addButton(QMessageBox::Close);
    msgbox.exec();

    return true;
}

//
// Return whether we use GPU.
//
bool is_gpu_accelerated(void)
{
	return true;
}

//
// Return whether we use OpenGL (for BGRA pixel order)
//
bool is_opengl_enabled(void)
{
	return true;
}

//
// Lock a GPU texture for an image.
//
bool lock_texture(int width, int height, pixel_t *pixels,
		  pixel_t **locked_pixels, void **texture)
{
    if (!opengl_lock_texture(width, height, pixels, locked_pixels, texture))
        return false;

    return true;
}

//
// Unlock a GPU texture for an image.
//
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture)
{
    opengl_unlock_texture(width, height, pixels, locked_pixels,
                          texture);
}

//
// Destroy a GPU texture for an image.
//
void destroy_texture(void *texture)
{
    opengl_destroy_texture(texture);
}

//
// Render an image to screen using normal shader.
//
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
    opengl_render_image(dst_left, dst_top, src_image, width, height,
                        src_left, src_top, alpha, bt);
}

//
// Render an image to screen with dim shader.
//
void render_image_dim(int dst_left, int dst_top,
		      struct image * RESTRICT src_image,
		      int width, int height, int src_left, int src_top)
{
    opengl_render_image_dim(dst_left, dst_top, src_image,
                            width, height, src_left, src_top);
}

//
// Render an image to screen with rule shader.
//
void render_image_rule(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
    opengl_render_image_rule(src_img, rule_img, threshold);
}

//
// Render an image to screen with melt shader.
//
void render_image_melt(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
    opengl_render_image_melt(src_img, rule_img, threshold);
}

//
// Make a sav directory.
//
bool make_sav_dir(void)
{
#ifdef OSX
#error "Not implemented for macOS"
#endif

    if (QDir(SAVE_DIR).exists())
        return true;

    QDir dir;
    if (!dir.mkdir(SAVE_DIR))
        return false;

    return true;
}

//
// Make a valid path for a file in a directory.
//
char *make_valid_path(const char *dir, const char *fname)
{
#ifdef OSX
#error "Not implemented for macOS"
#endif
	if (dir == NULL)
		dir = "";

	// パスのメモリを確保する
	size_t len = strlen(dir) + 1 + strlen(fname) + 1;
	char *buf = (char *)malloc(len);
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

//
// Reset a lap timer.
//
void reset_stop_watch(stop_watch_t *t)
{
    uint64_t ms = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    *t = ms;
}

//
// Get a lap from a lap timer.
//
int get_stop_watch_lap(stop_watch_t *t)
{
    uint64_t ms = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    return ms - *t;
}

//
// Show an exit confirmation.
//
bool exit_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

//
// Show a go-to-title confirmation.
//
bool title_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

//
// Show a delete confirmation.
//
bool delete_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

//
// Show an overwrite confirmation.
//
bool overwrite_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

//
// Show a reset confirmation.
//
bool default_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

//
// Play a video.
//
bool play_video(const char *fname, bool is_skippable)
{
    // TODO:

    // char *path;
    // path = make_valid_path(MOV_DIR, fname);
    // is_gst_playing = true;
    // is_gst_skippable = is_skippable;
    // gstplay_play(path, window);
    // free(path);
    return true;
}

//
// Stop a video.
//
void stop_video(void)
{
    // TODO:

    //gstplay_stop();
    //is_gst_playing = false;
}

//
// Return whether a video is playing.
//
bool is_video_playing(void)
{
    // TODO:

    //return is_gst_playing;
    return false;
}

//
// Update a window title.
//
void update_window_title(void)
{
    // TODO:
}

//
// Return whether we support full screen mode.
//
bool is_full_screen_supported(void)
{
    // We don't use full screen mode for a debugger.
    return false;
}

//
// Return whether we are in full screen mode.
//
bool is_full_screen_mode(void)
{
    // We don't use full screen mode for a debugger.
    return false;
}

//
// Start full screen mode.
//
void enter_full_screen_mode(void)
{
    // We don't use full screen mode for a debugger.
}

//
// Exit full screen mode.
//
void leave_full_screen_mode(void)
{
    // We don't use full screen mode for a debugger.
}

//
// Get a system locale.
//
const char *get_system_locale(void)
{
    const char *locale = QLocale().name().toUtf8().data();
    if (locale == NULL)
        return "en";
    else if (locale[0] == '\0' || locale[1] == '\0')
        return "en";
    else if (strncmp(locale, "en", 2) == 0)
        return "en";
    else if (strncmp(locale, "fr", 2) == 0)
        return "fr";
    else if (strncmp(locale, "de", 2) == 0)
        return "fr";
    else if (strncmp(locale, "it", 2) == 0)
        return "it";
    else if (strncmp(locale, "es", 2) == 0)
        return "es";
    else if (strncmp(locale, "el", 2) == 0)
        return "el";
    else if (strncmp(locale, "ru", 2) == 0)
        return "ru";
    else if (strncmp(locale, "zh_CN", 5) == 0)
        return "zh";
    else if (strncmp(locale, "zh_TW", 5) == 0)
        return "tw";
    else if (strncmp(locale, "ja", 2) == 0)
        return "ja";

    return "other";
}
/*
//
// Start a sound stream playing.
//
bool play_sound(int stream, struct wave *w)
{
    UNUSED_PARAMETER(stream);
    UNUSED_PARAMETER(w);
    return true;
}

//
// Stop a sound stream playing.
//
bool stop_sound(int stream)
{
    UNUSED_PARAMETER(stream);
    return true;
}

//
// Set a sound volume.
//
bool set_sound_volume(int stream, float vol)
{
    UNUSED_PARAMETER(stream);
    UNUSED_PARAMETER(vol);
    return true;
}

//
// Return whether a sound stream is finished.
//
bool is_sound_finished(int stream)
{
    UNUSED_PARAMETER(stream);
    return true;
}
*/

/*
 * A debugger HAL (platform.h API) implementation for Qt.
 */

//
// Check if the resume button was pressed.
//
bool is_resume_pushed(void)
{
    bool ret = MainWindow::obj->m_isResumePressed;
    MainWindow::obj->m_isResumePressed = false;
    return ret;
}

//
// Check if the next button was pressed.
//
bool is_next_pushed(void)
{
    bool ret = MainWindow::obj->m_isNextPressed;
    MainWindow::obj->m_isNextPressed = false;
    return ret;
}

//
// Check if the stop button was pressed.
//
bool is_pause_pushed(void)
{
    bool ret = MainWindow::obj->m_isPausePressed;
    MainWindow::obj->m_isPausePressed = false;
    return ret;
}

//
// Check if the script file name changed.
//
bool is_script_changed(void)
{
    // bool ret = isChangeScriptPressed;
    // isChangeScriptPressed = false;
    // return ret;
    return false;
}

//
// Get a script file name after a change.
//
const char *get_changed_script(void)
{
    // static char script[256];
    // snprintf(script, sizeof(script), "%s",
    //          [[debugWindowController getScriptName] UTF8String]);
    // return script;
    return "";
}

//
// Check if the line number to execute is changed.
//
bool is_line_changed(void)
{
    // bool ret = isLineChangePressed;
    // isLineChangePressed = false;
    // return ret;
    return 0;
}

//
// Get a line number after a change.
//
int get_changed_line(void)
{
    // return [debugWindowController getScriptLine];
    return 0;
}

//
// Check if the command string is updated.
//
bool is_command_updated(void)
{
    // bool ret = isCommandUpdatePressed;
    // isCommandUpdatePressed = false;
    // return ret;
    return false;
}

//
// Get a command string after a update.
//
const char *get_updated_command(void)
{
    // @autoreleasepool {
    //     static char command[4096];
    //     snprintf(command, sizeof(command), "%s",
    //              [[debugWindowController getCommandText] UTF8String]);
    //     return command;
    // }
    return "";
}

//
// Check if the script is reloaded.
//
bool is_script_reloaded(void)
{
    // bool ret = isReloadPressed;
    // isReloadPressed = false;
    // return ret;
    return false;
}

//
// Set the command running state.
//
void set_running_state(bool running, bool request_stop)
{
    // 実行状態を保存する
    MainWindow::obj->m_isRunning = running;

    // 停止によりコマンドの完了を待機中のとき
    if(request_stop) {
        MainWindow::obj->setWaitingState();
        return;
    }

    // 実行中のとき 
    if(running) {
        MainWindow::obj->setRunningState();
        return;
    }

    // 完全に停止中のとき
    MainWindow::obj->setStoppedState();
}

//
// デバッグ情報を更新する
//
void update_debug_info(bool script_changed)
{
/*
    [debugWindowController setScriptName:nsstr(get_script_file_name())];
    [debugWindowController setScriptLine:get_line_num()];
    [debugWindowController setCommandText:nsstr(get_line_string())];

    if (script_changed)
        [debugWindowController updateScriptTableView];

    [debugWindowController scrollScriptTableView];
    
	if (check_variable_updated() || script_changed)
		[debugWindowController updateVariableTextField];
*/
}

}; // extern "C"
