/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023-09-07 Created.
 *  - 2023-12-26 Changed audio output to ALSA.
 */

// Suika2 Base
extern "C" {
#include "suika.h"
};

// Suika2 Pro
extern "C" {
#include "package.h"
};

// HAL: graphics
extern "C" {
#include "glrender.h"
};

// HAL: sound
#ifndef USE_QT_AUDIO
extern "C" {
#include "asound.h"
};
#endif

// Qt6
#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Standard C++
#include <chrono>

// The sole instance of MainWindow.
MainWindow *MainWindow::obj;

//
// MainWindow class
//

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
    m_isScriptOpened = false;
    m_isExecLineChanged = false;

    // Determine the language to use.
    m_isEnglish = !QLocale().name().startsWith("ja");

#ifndef USE_QT_AUDIO
    // Initialize sound.
    init_asound();
#else
    // Setup the sound outputs.
    QAudioFormat format;
    format.setSampleFormat(QAudioFormat::Int16);
    format.setChannelCount(2);
    format.setSampleRate(44100);
    if (!QSysInfo::kernelVersion().contains("WSL2")) {
        for (int i = 0; i < MIXER_STREAMS; i++) {
            m_wave[i] = NULL;
            m_waveFinish[i] = false;
            m_soundSink[i] = new QAudioSink(format);
            m_soundDevice[i] = m_soundSink[i]->start();
        }
    } else {
        for (int i = 0; i < MIXER_STREAMS; i++) {
            m_wave[i] = NULL;
            m_waveFinish[i] = false;
            m_soundSink[i] = NULL;
            m_soundDevice[i] = NULL;
        }
    }
#endif

    // Setup a 33ms timer for game frames.
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->start(33);

    // Set the initial status to "stopped".
    setStoppedState();

    // Set button and labels texts.
    ui->continueButton->setText(m_isEnglish ? "Continue" : "続ける");
    ui->nextButton->setText(m_isEnglish ? "Next" : "次へ");
    ui->moveButton->setText(m_isEnglish ? "Move" : "移動");
    ui->stopButton->setText(m_isEnglish ? "Stop" : "停止");
    ui->errorButton->setText(m_isEnglish ? "Search error" : "次のエラー");
    ui->variableLabel->setText(m_isEnglish ? "Variables (non-initial value):" : "変数一覧(初期値0でないもの):");
    ui->writeButton->setText(m_isEnglish ? "Write Vars" : "変数の更新");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// The timer callback for game frames.
void MainWindow::onTimer()
{
    // Do a game frame.
    ui->openGLWidget->update();

#ifdef USE_QT_AUDIO
    const int SNDBUFSIZE = 4096;
    static uint32_t soundBuf[SNDBUFSIZE];

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
#endif
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    int frameWidth = event->size().width();
    int frameHeight = event->size().height();

    int panelWidth = 440;
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

void MainWindow::on_moveButton_clicked()
{
    updateScriptModelFromText();

    QTextCursor cursor = ui->scriptView->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    int lines = 1;
    while (cursor.positionInBlock()>0) {
        cursor.movePosition(QTextCursor::Up);
        lines++;
    }
    QTextBlock block = cursor.block().previous();
    while (block.isValid()) {
        lines += block.lineCount();
        block = block.previous();
    }

    m_changedExecLine = lines;
    m_isExecLineChanged = true;

    save_script();
}

void MainWindow::on_stopButton_clicked()
{
    m_isPausePressed = true;
}

void MainWindow::on_openScriptFileButton_clicked()
{
    // TODO: Open a dialog.
}

void MainWindow::on_scriptView_textChanged()
{
}

void MainWindow::on_writeButton_clicked()
{
    // Get a text from the vars text box.
    char buf[4096];
    strncpy(&buf[0], ui->variableTextEdit->toPlainText().toUtf8().data(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // Parse the text.
    char *p = buf;
    while(*p) {
        // Skip an empty line.
        if(*p == '\n') {
            p++;
            continue;
        }

        // Search for a start char of a next line.
        char *next_line = p;
        while(*next_line) {
            if(*next_line == '\n') {
                *next_line++ = '\0';
                break;
            }
            next_line++;
        }

        // Parse.
        int index, val;
        if(sscanf(p, "$%d=%d", &index, &val) != 2)
            index = -1, val = -1;
        if(index >= LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
            index = -1;

        // Set a variable value.
        if(index != -1)
            set_variable(index, val);

        // Move to a next line.
        p = next_line;
    }

    updateVariableText();
}

void MainWindow::on_errorButton_clicked()
{
    QString text = ui->scriptView->toPlainText();
    int lineCount = text.split(u8'\n').count();
    int cursorLine = getCursorLine();

    // Start searching from a line at (current-selected + 1).
    for (int i = cursorLine + 1; i < lineCount; i++) {
        const char *text = get_line_string_at_line_num(i);
        if (text[0] == '!') {
            setExecLine(i);
            return;
        }
    }

    // Don't re-start search if the selected item is at index 0.
    if (cursorLine == 0) {
        // Re-start searching from index 0 to index start.
        for (int i = 0; i <= cursorLine; i++) {
            const char *text = get_line_string_at_line_num(i);
            if (text[0] == '!') {
                setExecLine(i);
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

    // Disable the script open button.
    ui->openScriptButton->setEnabled(false);

    // Enable the script view.
    ui->scriptView->setEnabled(true);

    // Disable the search-error button.
    ui->errorButton->setEnabled(false);

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

    // Disable the script open button.
    ui->openScriptButton->setEnabled(false);

    // Enable the script view.
    ui->scriptView->setEnabled(true);

    // Disable the search-error button.
    ui->errorButton->setEnabled(false);

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

    // Enable the script open button.
    ui->openScriptButton->setEnabled(true);

    // Enable the script view.
    ui->scriptView->setEnabled(true);

    // Enable the search-error button.
    ui->errorButton->setEnabled(true);

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
    // 行を連列してスクリプト文字列を作成する
    QString text = "";
    for (int i = 0; i < get_line_count(); i++) {
        text.append(get_line_string_at_line_num(i));
        text.append("\n");
    }

    // テキストビューにテキストを設定する
    ui->scriptView->setText(text);
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
    setExecLine(get_expanded_line_num());
}

//
// Open a project.
//
void MainWindow::on_actionOpen_Project_triggered()
{
    // Open a project file.
    QString filename = QFileDialog::getOpenFileName(this, "Open", QString(), QObject::tr("Suika2 Project (*.suika2project)"), nullptr);
    if (filename.isEmpty())
        return;

    printf("%s\n", QDir(filename).dirName().toUtf8().data());

    // Set the current working directory.
    QDir::setCurrent(QDir(QFileInfo(filename).absoluteDir()).absolutePath());

    // Initialize.
    init_locale_code();
    if(!init_conf())
        abort();

    // Start game rendering.
    ui->openGLWidget->start();

    // FIXME: workaround
    resize(this->width() + 1, this->height());
}

//
// Export for Linux.
//
void MainWindow::on_actionExport_for_Linux_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export a Linux game?\n"
                   "This may take a while." :
                   "Linuxゲームをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Copy an export template.
    if (!copyExportTemplateWithGame("export-linux", true))
        return;

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("export-linux"));
}

//
// Export for Windows.
//
void MainWindow::on_actionExport_for_Windows_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export a Windows game?\n"
                   "This may take a while." :
                   "Windowsゲームをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Copy an export template.
    if (!copyExportTemplateWithGame("export-windows", true))
        return;

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("export-windows"));
}

//
// Export for macOS.
//
void MainWindow::on_actionExport_for_macOS_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export a macOS game?\n"
                   "This may take a while." :
                   "macOSゲームをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Copy an export template.
    if (!copyExportTemplateWithGame("export-macos", true))
        return;

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("export-macos"));
}

//
// Export for iOS.
//
void MainWindow::on_actionExport_for_iOS_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export an iOS game?\n"
                   "This may take a while." :
                   "iOSゲームをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Copy an export template.
    if (!copyExportTemplateWithGame("export-ios", true))
        return;

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("export-ios"));
}

//
// Export for Android.
//
void MainWindow::on_actionExport_for_Android_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export an Android game?\n"
                   "This may take a while." :
                   "Androidゲームをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Copy an export template.
    if (!copyExportTemplateWithGame("export-android", true))
        return;

    // Copy assets.
    const char *subdir[] = {"anime", "bg", "bgm", "cg", "ch", "conf", "cv",
                            "font", "gui", "mov", "rule", "se", "txt", "wms"};
    for (int i = 0; i < sizeof(subdir) / sizeof(const char *); i++) {
        QString src = subdir[i];
        QString dst = "export-android";
        copyFiles(src, dst);
    }

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("export-android"));
}

//
// Export for Web.
//
void MainWindow::on_actionExport_for_Web_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export a Web game?\n"
                   "This may take a while." :
                   "Webゲームをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Copy an export template.
    if (!copyExportTemplateWithGame("export-web", true))
        return;

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("export-web"));
}

//
// Export a package only.
//
void MainWindow::on_actionExport_package_only_triggered()
{
    QMessageBox msgbox(nullptr);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setWindowTitle("Export");
    msgbox.setText(m_isEnglish ?
                   "Are you sure you want to export a package file?\n"
                   "This may take a while." :
                   "パッケージファイルをエクスポートします。\n"
                   "この処理には時間がかかります。\n"
                   "よろしいですか？");
    msgbox.addButton(QMessageBox::Yes);
    msgbox.addButton(QMessageBox::No);
    if (msgbox.exec() != QMessageBox::Yes)
        return;

    // Generate a package in the current directory.
    if (!create_package("")) {
        log_info(m_isEnglish ?
                 "Failed to exported data01.arc" :
                 "data01.arcのエクスポートに失敗しました。");
        return;
    }

    // Open the folder.
    QDesktopServices::openUrl(QUrl::fromLocalFile("data01.arc"));
}

// Copy export-* files.
bool MainWindow::copyExportTemplateWithGame(const QString& name, bool copyArc)
{
    QDir src = QApplication::applicationDirPath();
    src.cd("../share/suika2/" + name);

    QDir dst(".");

    // Copy a template directory.
    copyFiles(src.path(), dst.path());

    // Copy an archive.
    if (copyArc)
        QFile::copy("data01.arc", dst.path() + QDir::separator() + "data01.arc");

    return true;
}

// Copy files recursively.
bool MainWindow::copyFiles(QString src, QString dst)
{
    QDir dir(src);
    if (!dir.exists())
        return false;

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copyFiles(src + QDir::separator() + d, dst_path);
    }

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }

    return true;
}

// Get the cursor line.
int MainWindow::getCursorLine()
{
    QTextCursor cursor = ui->scriptView->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    int lines = 1;
    while (cursor.positionInBlock()>0) {
        cursor.movePosition(QTextCursor::Up);
        lines++;
    }
    QTextBlock block = cursor.block().previous();
    while (block.isValid()) {
        lines += block.lineCount();
        block = block.previous();
    }
    return lines;
}

// Set execution line.
void MainWindow::setExecLine(int line)
{
    QTextCursor lineCursor(ui->scriptView->document()->findBlockByLineNumber(line));
    lineCursor.select(QTextCursor::LineUnderCursor);
    ui->scriptView->setTextCursor(lineCursor);
}

// Update script model from text.
void MainWindow::updateScriptModelFromText()
{
    // Reset parse errors, will notify the first error.
    dbg_reset_parse_error_count();

    // Get the text from text view and update script model for each line.
    QString text = ui->scriptView->toPlainText();
    QStringList lines = text.split(u'\n');
    int lineCount = 0;
    for (QString line : lines) {
		// Update a line.
        const char *utf8 = line.toUtf8();
		if (lineCount < get_line_count())
			update_script_line(lineCount, utf8);
		else
			insert_script_line(lineCount, utf8);

		lineCount++;
	}

	// Process removed lines.
	m_isExecLineChanged = false;
	for (int i = get_line_count() - 1; i >= lineCount; i--)
		if (delete_script_line(lineCount))
			m_isExecLineChanged = true;

	// Reparse if there are extended syntaxes.
	reparse_script_for_structured_syntax();

	// If failed to parse and reparse:
	if (dbg_get_parse_error_count() > 0) {
		// Set text again to reflect '!' at the line starts.
		updateScriptView();
	}
}

//
// Main HAL (See also src/hal.h)
//

extern "C" {

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

bool is_gpu_accelerated(void)
{
    return true;
}

bool is_opengl_enabled(void)
{
    return true;
}

void notify_image_update(struct image *img)
{
    opengl_notify_image_update(img);
}

void notify_image_free(struct image *img)
{
    opengl_notify_image_free(img);
}

void render_image_normal(int dst_left,
                         int dst_top,
                         int dst_width,
                         int dst_height,
                         struct image *src_image,
                         int src_left,
                         int src_top,
                         int src_width,
                         int src_height,
                         int alpha)
{
    opengl_render_image_normal(dst_left,
                               dst_top,
                               dst_width,
                               dst_height,
                               src_image,
                               src_left,
                               src_top,
                               src_width,
                               src_height,
                               alpha);
}

void render_image_add(int dst_left,
                      int dst_top,
                      int dst_width,
                      int dst_height,
                      struct image *src_image,
                      int src_left,
                      int src_top,
                      int src_width,
                      int src_height,
                      int alpha)
{
    opengl_render_image_add(dst_left,
                            dst_top,
                            dst_width,
                            dst_height,
                            src_image,
                            src_left,
                            src_top,
                            src_width,
                            src_height,
                            alpha);
}

void render_image_dim(int dst_left,
                      int dst_top,
                      int dst_width,
                      int dst_height,
                      struct image *src_image,
                      int src_left,
                      int src_top,
                      int src_width,
                      int src_height,
                      int alpha)
{
    opengl_render_image_dim(dst_left,
                            dst_top,
                            dst_width,
                            dst_height,
                            src_image,
                            src_left,
                            src_top,
                            src_width,
                            src_height,
                            alpha);
}

void render_image_rule(struct image *src_img, struct image *rule_img, int threshold)
{
    opengl_render_image_rule(src_img, rule_img, threshold);
}

void render_image_melt(struct image *src_img, struct image *rule_img, int progress)
{
    opengl_render_image_melt(src_img, rule_img, progress);
}

bool make_sav_dir(void)
{
    if (QDir(SAVE_DIR).exists())
        return true;
    QDir qdir;
    if (!qdir.mkdir(SAVE_DIR))
        return false;
    return true;
}

char *make_valid_path(const char *dir, const char *fname)
{
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
}

void reset_lap_timer(uint64_t *t)
{
    *t = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

uint64_t get_lap_timer_millisec(uint64_t *t)
{
    uint64_t ms = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    return ms - *t;
}

bool exit_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

bool title_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

bool delete_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

bool overwrite_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

bool default_dialog(void)
{
    // We don't show it for a debugger.
    return true;
}

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

void stop_video(void)
{
    // TODO:

    //gstplay_stop();
    //is_gst_playing = false;
}

bool is_video_playing(void)
{
    // TODO:

    //return is_gst_playing;
    return false;
}

void update_window_title(void)
{
    // TODO:
}

bool is_full_screen_supported(void)
{
    // We don't use full screen mode for a debugger.
    return false;
}

bool is_full_screen_mode(void)
{
    // We don't use full screen mode for a debugger.
    return false;
}

void enter_full_screen_mode(void)
{
    // We don't use full screen mode for a debugger.
}

void leave_full_screen_mode(void)
{
    // We don't use full screen mode for a debugger.
}

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
        return "de";
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
// We use ALSA by default instead of the sound support of Qt.
//
#ifdef USE_QTAUDIO
bool play_sound(int stream, struct wave *w)
{
    if (MainWindow::obj == NULL)
        return true;
    if (MainWindow::obj->m_soundSink[stream] == NULL)
        return true;

    if (MainWindow::obj->m_wave[stream] == NULL)
        MainWindow::obj->m_soundSink[stream]->start();

    MainWindow::obj->m_wave[stream] = w;
    MainWindow::obj->m_waveFinish[stream] = false;
    return true;
}

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

bool set_sound_volume(int stream, float vol)
{
    if (MainWindow::obj == NULL)
        return true;
    if (MainWindow::obj->m_soundSink[stream] == NULL)
        return true;

    MainWindow::obj->m_soundSink[stream]->setVolume(vol);
    return true;
}

bool is_sound_finished(int stream)
{
    if (MainWindow::obj == NULL)
        return true;
    if (MainWindow::obj->m_soundSink[stream] == NULL)
        return true;

    if (!MainWindow::obj->m_waveFinish[stream])
        return false;

    return true;
}
#endif

void speak_text(const char *text)
{
    UNUSED_PARAMETER(text);
}

//
// Pro HAL (See also src/pro.h)
//

bool is_continue_pushed(void)
{
    bool ret = MainWindow::obj->m_isResumePressed;
    MainWindow::obj->m_isResumePressed = false;
    return ret;
}

bool is_next_pushed(void)
{
    bool ret = MainWindow::obj->m_isNextPressed;
    MainWindow::obj->m_isNextPressed = false;
    return ret;
}

bool is_stop_pushed(void)
{
    bool ret = MainWindow::obj->m_isPausePressed;
    MainWindow::obj->m_isPausePressed = false;
    return ret;
}

bool is_script_opened(void)
{
    bool ret = MainWindow::obj->m_isScriptOpened;
    MainWindow::obj->m_isScriptOpened = false;
    return ret;
}

const char *get_opened_script(void)
{
    return MainWindow::obj->m_openedScript.toUtf8();
}

bool is_exec_line_changed(void)
{
    bool ret = MainWindow::obj->m_isExecLineChanged;
    MainWindow::obj->m_isExecLineChanged = false;
    return ret;
}

int get_changed_exec_line(void)
{
    return MainWindow::obj->m_changedExecLine;
}

void on_change_running_state(bool running, bool request_stop)
{
    // Save the running state.
    MainWindow::obj->m_isRunning = running;

    // If we are waiting for a stop by a request.
    if(request_stop) {
        MainWindow::obj->setWaitingState();
        return;
    }

    // If we are running.
    if(running) {
        MainWindow::obj->setRunningState();
        return;
    }

    // If we are stopping.
    MainWindow::obj->setStoppedState();
}

void on_load_script(void)
{
    MainWindow::obj->ui->fileNameTextEdit->setText(get_script_file_name());
    MainWindow::obj->updateScriptView();
}

void on_change_position(void)
{
    MainWindow::obj->scrollScript();
}

void on_update_variable(void)
{
    MainWindow::obj->updateVariableText();
}

}; // extern "C"
