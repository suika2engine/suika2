/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023/09/07 作成
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QCoreApplication>
#include <QStandardItemModel>
#include <QResizeEvent>
#include <QModelIndex>
#include <QMessageBox>
#include <QDir>
#include <QLocale>
#include <QAudioFormat>

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

    // Clear the status flags.
    m_isRunning = false;
    m_isResumePressed = false;
    m_isNextPressed = false;
    m_isPausePressed = false;
    m_isChangeScriptPressed = false;
    m_isChangeLinePressed = false;
    m_isCommandUpdatePressed = false;
    m_isReloadPressed = false;

    // Determine the language to use.
    m_isEnglish = !QLocale().name().startsWith("ja");

    // Setup the sound outputs.
    QAudioFormat format;
    format.setSampleFormat(QAudioFormat::Int16);
    format.setChannelCount(2);
    format.setSampleRate(44100);
    for (int i = 0; i < MIXER_STREAMS; i++) {
        m_wave[i] = NULL;
        m_waveFinish[i] = false;
        m_soundSink[i] = new QAudioSink(format);
        m_soundDevice[i] = m_soundSink[i]->start();
    }

    // Setup a 33ms timer for game frames.
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->start(33);

    // Set the initial status to "stopped".
    setStoppedState();

    // Set button and labels texts.
    ui->fileNameLabel->setText(m_isEnglish ? "Script file name:" : "スクリプトファイル名:");
    ui->lineNumberLabel->setText(m_isEnglish ? "Script line number:" : "スクリプト行番号:");
    ui->commandLabel->setText(m_isEnglish ? "Script command to be executed:" : "実行されるコマンド行:");
    ui->scriptContentLabel->setText(m_isEnglish ? "Script content:" : "スクリプトリスト:");
    ui->scriptContentLabel->setText(m_isEnglish ? "Script content:" : "スクリプトリスト:");
    ui->variableLabel->setText(m_isEnglish ? "Variables (non-initial value):" : "変数一覧(初期値0でないもの):");
    ui->continueButton->setText(m_isEnglish ? "Continue" : "続ける");
    ui->nextButton->setText(m_isEnglish ? "Next" : "次へ");
    ui->stopButton->setText(m_isEnglish ? "Stop" : "停止");
    ui->updateScriptButton->setText(m_isEnglish ? "Update" : "更新");
    ui->updateLineNumberButton->setText(m_isEnglish ? "Update" : "更新");
    ui->updateCommandButton->setText(m_isEnglish ? "Update" : "更新");
    ui->resetCommandButton->setText(m_isEnglish ? "Reset" : "リセット");
    ui->errorButton->setText(m_isEnglish ? "Search error" : "次のエラー");
    ui->overwriteButton->setText(m_isEnglish ? "Overwrite" : "上書き保存");
    ui->reloadButton->setText(m_isEnglish ? "Reload" : "再読み込み");
    ui->writeButton->setText(m_isEnglish ? "Update variables" : "変数の更新");
}

MainWindow::~MainWindow()
{
    delete ui;

    // Destroy the sound outputs.
    for (int i = 0; i < MIXER_STREAMS; i++) {
        if (m_soundDevice[i] != NULL) {
            m_soundDevice[i] = NULL;
        }
        if (m_soundSink[i] != NULL) {
            m_soundSink[i]->stop();
            delete m_soundSink[i];
            m_soundSink[i] = NULL;
        }
    }
}

// The timer callback for game frames.
void MainWindow::onTimer()
{
    const int SNDBUFSIZE = 4096;
    static uint32_t soundBuf[SNDBUFSIZE];

    // Do a game frame.
    ui->openGLWidget->update();

    // Do sound bufferings.
    for (int i = 0; i < MIXER_STREAMS; i++) {
        if (m_wave[i] == NULL)
            continue;

        int needSamples = m_soundSink[i]->bytesFree() / 4;
        int restSamples = needSamples;
        while (restSamples > 0) {
            int reqSamples = restSamples > SNDBUFSIZE ? SNDBUFSIZE : restSamples;
            int readSamples = get_wave_samples(m_wave[i], &soundBuf[0], reqSamples);
            if (readSamples == 0) {
                m_wave[i] = NULL;
                m_waveFinish[i] = true;
                break;
            }
            m_soundDevice[i]->write((char const *)&soundBuf[0], readSamples * 4);
            restSamples -= readSamples;
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    int frameWidth = event->size().width();
    int frameHeight = event->size().height();

    int panelWidth = 340;
    int panelHeight = 720;

    ui->openGLWidget->resize(frameWidth - panelWidth, frameHeight);
    ui->controlPanel->move(frameWidth - panelWidth + 5, 5);
}

void MainWindow::on_continueButton_clicked()
{
    m_isResumePressed = true;
}

void MainWindow::on_nextButton_clicked()
{
    m_isNextPressed = true;
}

void MainWindow::on_stopButton_clicked()
{
    m_isPausePressed = true;
}

void MainWindow::on_fileNameEdit_returnPressed()
{
    m_isChangeScriptPressed = true;
}

void MainWindow::on_lineNumberEdit_returnPressed()
{
    m_isChangeLinePressed = true;
}

void MainWindow::on_updateLineNumberButton_clicked()
{
    m_isChangeLinePressed = true;
}

void MainWindow::on_updateScriptFileButton_clicked()
{
    m_isChangeScriptPressed = true;
}

void MainWindow::on_openScriptFileButton_clicked()
{
    // TODO: Open a dialog.
}

void MainWindow::on_updateCommandButton_clicked()
{
    m_isCommandUpdatePressed = true;
}

void MainWindow::on_resetCommandButton_clicked()
{
    ui->commandEdit->setText(get_line_string());
}

void MainWindow::on_scriptListView_doubleClicked(const QModelIndex &index)
{
    int line = ui->scriptListView->currentIndex().row();
    QString text = QString::number(line);
    ui->lineNumberEdit->setText(text);
    m_isChangeLinePressed = true;
}

void MainWindow::on_writeButton_clicked()
{
    // テキストボックスの内容を取得する
    char buf[4096];
    strncpy(&buf[0], ui->variableTextEdit->toPlainText().toUtf8().data(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // パースする
    char *p = buf;
    while(*p) {
        // 空行を読み飛ばす
        if(*p == '\n') {
            p++;
            continue;
        }

        // 次の行の開始文字を探す
        char *next_line = p;
        while(*next_line) {
            if(*next_line == '\n') {
                *next_line++ = '\0';
                break;
            }
            next_line++;
        }

        // パースする
        int index, val;
        if(sscanf(p, "$%d=%d", &index, &val) != 2)
            index = -1, val = -1;
        if(index >= LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
            index = -1;

        // 変数を設定する
        if(index != -1)
            set_variable(index, val);

        // 次の行へポインタを進める
        p = next_line;
    }

    updateVariableText();
}

void MainWindow::on_reloadButton_clicked()
{
    m_isReloadPressed = true;
}

void MainWindow::on_overwriteButton_clicked()
{
    const char *scr = get_script_file_name();
    if(strcmp(scr, "DEBUG") == 0)
        return;

    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Suika2 Pro");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to overwrite the script file?" :
                   "スクリプトファイルを上書き保存します。\nよろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;
    
    char *path = make_valid_path(SCRIPT_DIR, scr);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        free(path);

        QMessageBox errmsg(nullptr);
        errmsg.setIcon(QMessageBox::Critical);
        errmsg.setWindowTitle("Suika2 Pro");
        errmsg.setText(m_isEnglish ? "Cannot write to file." : "ファイルに書き込めません。");
        errmsg.addButton(QMessageBox::Close);
        errmsg.exec();
        return;
    }
    free(path);

    for (int i = 0; i < get_line_count(); i++) {
        int body = fputs(get_line_string_at_line_num(i), fp);
        int crlf = fputs("\n", fp);
        if(body < 0 || crlf < 0) {
            fclose(fp);
            return;
	}
    }
    fclose(fp);
}

void MainWindow::on_errorButton_clicked()
{
    int lines = get_line_count();
    int start = ui->scriptListView->currentIndex().column();

    // Start searching from a line at (current-selected + 1).
    for(int i = start + 1; i < lines; i++) {
        const char *text = get_line_string_at_line_num(i);
        if(text[0] == '!') {
            QModelIndex cellIndex = ui->scriptListView->model()->index(i, 0);
            ui->scriptListView->setCurrentIndex(cellIndex);
            return;
	}
    }

    // Don't re-start search if the selected item is at index 0.
    if(start == 0) {
        // Re-start searching from index 0 to index start.
        for(int i = 0; i <= start; i++) {
            const char *text = get_line_string_at_line_num(i);
            if(text[0] == '!') {
                QModelIndex cellIndex = ui->scriptListView->model()->index(i, 0);
                ui->scriptListView->setCurrentIndex(cellIndex);
                return;
            }
        }
    }

    // Show a dialog if no error.
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Suika2 Pro");
    msgbox.setText(m_isEnglish ? "No error." : "エラーはありません。");
    msgbox.addButton(QMessageBox::Close);
    msgbox.exec();
}

// Set a view state for when we are waiting for a command finish by a pause.
void MainWindow::setWaitingState()
{
    // Set the window title.
    setWindowTitle(m_isEnglish ? "Waiting for command finish..." : "コマンドの完了を待機中...");

    // Disable the continue button.
    ui->continueButton->setEnabled(false);
    ui->continueButton->setText(m_isEnglish ? "Resume" : "続ける");

    // Disable the next button.
    ui->nextButton->setEnabled(false);
    ui->nextButton->setText(m_isEnglish ? "Next" : "次へ");

    // Disable the stop button.
    ui->stopButton->setEnabled(false);
    ui->stopButton->setText(m_isEnglish ? "Pause" : "停止");

    // Disable the script file text field.
    ui->fileNameTextEdit->setEnabled(false);

    // Disable the script file update button.
    ui->updateScriptButton->setEnabled(false);

    // Disable the script open button.
    ui->openScriptButton->setEnabled(false);

    // Set the line number label.
    ui->lineNumberLabel->setText(m_isEnglish ? "Current Waiting Line:" : "現在完了待ちの行番号:");

    // Disable the line number text field.
    ui->lineNumberEdit->setEnabled(false);

    // Disable the line number update button.
    ui->updateLineNumberButton->setEnabled(false);

    // Set the command label.
    ui->commandLabel->setText(m_isEnglish ? "Current Waiting Command:" : "現在完了待ちのコマンド:");

    // Disable the command text edit.
    ui->commandEdit->setEnabled(false);

    // Disable the command update button.
    ui->updateCommandButton->setEnabled(false);

    // Disable the command reset button.
    ui->resetCommandButton->setEnabled(false);

    // Enable the script view.
    ui->scriptListView->setEnabled(true);

    // Disable the search-error button.
    ui->errorButton->setEnabled(false);

    // Disable the overwrite button.
    ui->overwriteButton->setEnabled(false);

    // Disable the reload button.
    ui->reloadButton->setEnabled(false);

    // Disable the variable text edit.
    ui->variableTextEdit->setEnabled(false);

    // Disable the variable write button.
    ui->writeButton->setEnabled(false);

    // TODO: disable the open-script menu item.
    // TODO: disable the overwrite menu item.
    // TODO: disable the export menu item.
    // TODO: disable the continue menu item.
    // TODO: disable the next menu item.
    // TODO: disable the stop menu item.
    // TODO: disable the search-error menu item.
    // TODO: disable the reload menu item.
}

// Set a view state for when we are running a command.
void MainWindow::setRunningState()
{
    // Set the window title.
    setWindowTitle(m_isEnglish ? "Running..." : "実行中...");

    // Disable the continue button.
    ui->continueButton->setEnabled(false);
    ui->continueButton->setText(m_isEnglish ? "Resume" : "続ける");

    // Disable the next button.
    ui->nextButton->setEnabled(false);
    ui->nextButton->setText(m_isEnglish ? "Next" : "次へ");

    // Enable the stop button.
    ui->stopButton->setEnabled(true);
    ui->stopButton->setText(m_isEnglish ? "Pause" : "停止");

    // Disable the script file text field.
    ui->fileNameTextEdit->setEnabled(false);

    // Disable the script file update button.
    ui->updateScriptButton->setEnabled(false);

    // Disable the script open button.
    ui->openScriptButton->setEnabled(false);

    // Set the line number label.
    ui->lineNumberLabel->setText(m_isEnglish ? "Current Waiting Line:" : "現在完了待ちの行番号:");

    // Disable the line number text field.
    ui->lineNumberEdit->setEnabled(false);

    // Disable the line number update button.
    ui->updateLineNumberButton->setEnabled(false);

    // Set the command label.
    ui->commandLabel->setText(m_isEnglish ? "Current Waiting Command:" : "現在完了待ちのコマンド:");

    // Disable the command text edit.
    ui->commandEdit->setEnabled(false);

    // Disable the command update button.
    ui->updateCommandButton->setEnabled(false);

    // Disable the command reset button.
    ui->resetCommandButton->setEnabled(false);

    // Enable the script view.
    ui->scriptListView->setEnabled(true);

    // Disable the search-error button.
    ui->errorButton->setEnabled(false);

    // Disable the overwrite button.
    ui->overwriteButton->setEnabled(false);

    // Disable the reload button.
    ui->reloadButton->setEnabled(false);

    // Disable the variable text edit.
    ui->variableTextEdit->setEnabled(false);

    // Disable the variable write button.
    ui->writeButton->setEnabled(false);

    // TODO: disable the open-script menu item.
    // TODO: disable the overwrite menu item.
    // TODO: disable the export menu item.
    // TODO: disable the continue menu item.
    // TODO: disable the next menu item.
    // TODO: enable the stop menu item.
    // TODO: disable the search-error menu item.
    // TODO: disable the reload menu item.
}

// Set a view state for when we are completely pausing.
void MainWindow::setStoppedState()
{
    // Set the window title.
    setWindowTitle(m_isEnglish ? "Stopped" : "停止中");

    // Enable the continue button.
    ui->continueButton->setEnabled(true);
    ui->continueButton->setText(m_isEnglish ? "Resume" : "続ける");

    // Enable the next button.
    ui->nextButton->setEnabled(true);
    ui->nextButton->setText(m_isEnglish ? "Next" : "次へ");

    // Disable the stop button.
    ui->stopButton->setEnabled(false);
    ui->stopButton->setText(m_isEnglish ? "Pause" : "停止");

    // Enable the script file text field.
    ui->fileNameTextEdit->setEnabled(true);

    // Enable the script file update button.
    ui->updateScriptButton->setEnabled(true);

    // Enable the script open button.
    ui->openScriptButton->setEnabled(true);

    // Enable the script open button.
    ui->openScriptButton->setEnabled(false);

    // Set the line number label.
    ui->lineNumberLabel->setText(m_isEnglish ? "Next Line to be Executed:" : "次に実行される行番号:");

    // Enable the line number text field.
    ui->lineNumberEdit->setEnabled(true);

    // Enable the line number update button.
    ui->updateLineNumberButton->setEnabled(true);

    // Set the command label.
    ui->commandLabel->setText(m_isEnglish ? "Next Command to be Executed:" : "次に実行されるコマンド:");

    // Enable the command text edit.
    ui->commandEdit->setEnabled(true);

    // Enable the command update button.
    ui->updateCommandButton->setEnabled(true);

    // Disable the command reset button.
    ui->resetCommandButton->setEnabled(true);

    // Enable the script view.
    ui->scriptListView->setEnabled(true);

    // Enable the search-error button.
    ui->errorButton->setEnabled(true);

    // Enable the overwrite button.
    ui->overwriteButton->setEnabled(true);

    // Enable the reload button.
    ui->reloadButton->setEnabled(true);

    // Enable the variable text edit.
    ui->variableTextEdit->setEnabled(true);

    // Enable the variable write button.
    ui->writeButton->setEnabled(true);

    // TODO: enable the open-script menu item.
    // TODO: enable the overwrite menu item.
    // TODO: enable the export menu item.
    // TODO: enable the continue menu item.
    // TODO: enable the next menu item.
    // TODO: disable the stop menu item.
    // TODO: enable the search-error menu item.
    // TODO: enable the reload menu item.
}

//
// Update the script view.
//
void MainWindow::updateScriptView()
{
    QAbstractItemModel *oldModel = ui->scriptListView->model();
    QStandardItemModel *newModel = new QStandardItemModel();

    int count = get_line_count();
    for (int lineNum = 0; lineNum < count; lineNum++) {
        QStandardItem *item = new QStandardItem();
        item->setText(::get_line_string_at_line_num(lineNum));
        item->setEditable(false);
        newModel->appendRow(item);
    }
    ui->scriptListView->setModel(newModel);

    delete oldModel;
}

//
// Update the variable text.
//
void MainWindow::updateVariableText()
{
    QString text = "";
    for(int index = 0; index < LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE; index++) {
        // If a variable has an initial value and has not been changed, skip.
        int val = get_variable(index);
        if(val == 0 && !is_variable_changed(index))
            continue;

        // Add a line.
        text += QString("$%1=%2\n").arg(index).arg(val);
    }

    // Set to the text edit.
    ui->variableTextEdit->setText(text);
}

//
// Scroll script view.
//
void MainWindow::scrollScript()
{
    int line = get_expanded_line_num();
    QModelIndex cellIndex = ui->scriptListView->model()->index(line, 0);
    ui->scriptListView->setCurrentIndex(cellIndex);
    ui->scriptListView->scrollTo(cellIndex);
}

//
// Export data01.arc
//
void MainWindow::on_actionExport_data01_arc_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
				   "Are you sure you want to export the package file?\n"
				   "This may take a while." :
				   "パッケージをエクスポートします。\n"
				   "この処理には時間がかかります。\n"
				   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Get the game directory.
    char *gamePath = make_valid_path(NULL, NULL);
    if (gamePath == NULL) {
        log_memory();
        return;
    }

	// Generate a package.
    if (create_package(gamePath)) {
		log_info(m_isEnglish ?
				 "Successfully exported data01.arc" :
				 "data01.arcのエクスポートに成功しました。");
	}
    free(gamePath);
}

//
// Export Web version
//
void MainWindow::on_actionExport_for_Web_triggered()
{

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
#if defined(OSX)
    QDir qdir(QCoreApplication::applicationDirPath()); // dir points to /XXX/suika.app/Contents/MacOS
    qdir.cdUp(); // dir points to /XXX/suika.app/Contents
    qdir.cdUp(); // dir points to /XXX/suika.app
    qdir.cdUp(); // dir points to /XXX
    QString path = qdir.currentPath() + QString("/") + QString(SAVE_DIR);
    if (QDir(path).exists())
        return true;
    QDir mdir;
    if (!mdir.mkdir(path))
        return false;
    return true;
#else
    if (QDir(SAVE_DIR).exists())
        return true;
    QDir qdir;
    if (!qdir.mkdir(SAVE_DIR))
        return false;
    return true;
#endif
}

//
// Make a valid path for a file in a directory.
//
char *make_valid_path(const char *dir, const char *fname)
{
#if defined(OSX)
    QDir qdir(QCoreApplication::applicationDirPath()); // dir points to /XXX/suika.app/Contents/MacOS
    qdir.cdUp(); // dir points to /XXX/suika.app/Contents
    qdir.cdUp(); // dir points to /XXX/suika.app
    qdir.cdUp(); // dir points to /XXX
    QString path = qdir.currentPath();
    if (dir != NULL)
        path += QString("/") + QString(dir);
    if (fname != NULL)
        path += QString("/") + QString(fname);
    char *ret = strdup(path.toUtf8().data());
    if (ret == NULL) {
        log_memory();
        return NULL;
    }
    return ret;
#elif defined(WIN)
    QDir qdir(QCoreApplication::applicationDirPath());
    QString path = qdir.currentPath();
    if (dir != NULL)
        path += QString("\\") + QString(dir);
    if (fname != NULL)
        path += QString("\\") + QString(fname);
    char *ret = strdup(path.toUtf8().data());
    if (ret == NULL) {
        log_memory();
        return NULL;
    }
    return ret;
#else
    QDir qdir(QCoreApplication::applicationDirPath());
    QString path = qdir.currentPath();
    if (dir != NULL)
        path += QString("/") + QString(dir);
    if (fname != NULL)
        path += QString("/") + QString(fname);
    char *ret = strdup(path.toUtf8().data());
    if (ret == NULL) {
        log_memory();
        return NULL;
    }
    return ret;
#endif
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

//
// Start a sound stream playing.
//
bool play_sound(int stream, struct wave *w)
{
    if (MainWindow::obj == NULL)
        return true;
    if (MainWindow::obj->m_soundSink[stream] == NULL)
        return true;
    bool isPlaying = MainWindow::obj->m_wave[stream] != NULL;
    if (!isPlaying)
        MainWindow::obj->m_soundSink[stream]->start();
    MainWindow::obj->m_wave[stream] = w;
    MainWindow::obj->m_waveFinish[stream] = false;
    return true;
}

//
// Stop a sound stream playing.
//
bool stop_sound(int stream)
{
    if (MainWindow::obj == NULL)
        return true;
    if (MainWindow::obj->m_soundSink[stream] == NULL)
        return true;
    MainWindow::obj->m_soundSink[stream]->stop();
    MainWindow::obj->m_wave[stream] = NULL;
    MainWindow::obj->m_waveFinish[stream] = false;
    return true;
}

//
// Set a sound volume.
//
bool set_sound_volume(int stream, float vol)
{
    if (MainWindow::obj == NULL)
        return true;
    if (MainWindow::obj->m_soundSink[stream] == NULL)
        return true;
    MainWindow::obj->m_soundSink[stream]->setVolume(vol);
    return true;
}

//
// Return whether a sound stream is finished.
//
bool is_sound_finished(int stream)
{
    if (MainWindow::obj == NULL)
        return true;
    return MainWindow::obj->m_waveFinish[stream];
}

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
    bool ret = MainWindow::obj->m_isChangeScriptPressed;
    MainWindow::obj->m_isChangeScriptPressed = false;
    return ret;
}

//
// Get a script file name after a change.
//
const char *get_changed_script(void)
{
    static char script[256];
    snprintf(script, sizeof(script), "%s", MainWindow::obj->ui->fileNameTextEdit->text().toUtf8().data());
    return script;
}

//
// Check if the line number to execute is changed.
//
bool is_line_changed(void)
{
    bool ret = MainWindow::obj->m_isChangeLinePressed;
    MainWindow::obj->m_isChangeLinePressed = false;
    return ret;
}

//
// Get a line number after a change.
//
int get_changed_line(void)
{
    return MainWindow::obj->ui->lineNumberEdit->text().toInt();
}

//
// Check if the command string is updated.
//
bool is_command_updated(void)
{
    bool ret = MainWindow::obj->m_isCommandUpdatePressed;
    MainWindow::obj->m_isCommandUpdatePressed = false;
    return ret;
}

//
// Get a command string after a update.
//
const char *get_updated_command(void)
{
    static char command[4096];
    snprintf(command, sizeof(command), "%s", MainWindow::obj->ui->commandEdit->document()->toPlainText().toUtf8().data());
    return command;
}

//
// Check if the script is reloaded.
//
bool is_script_reloaded(void)
{
    bool ret = MainWindow::obj->m_isReloadPressed;
    MainWindow::obj->m_isReloadPressed = false;
    return ret;
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
    MainWindow::obj->ui->fileNameTextEdit->setText(get_script_file_name());
    MainWindow::obj->ui->lineNumberEdit->setText(QString::number(get_expanded_line_num()));
    MainWindow::obj->ui->commandEdit->setText(get_line_string());
    if (script_changed)
        MainWindow::obj->updateScriptView();
    if (check_variable_updated() || script_changed)
        MainWindow::obj->updateVariableText();
    MainWindow::obj->scrollScript();
}

}; // extern "C"

