#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QTimer>
#include <QStandardItemModel>
#include <QResizeEvent>
#include <QModelIndex>
#include <QMessageBox>
#include <QDir>
#include <QLocale>
#include <QTextBlock>

// We use ALSA directly on Linux because Qt doesn't support ALSA.
#ifdef USE_QT_AUDIO
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void resizeEvent(QResizeEvent *event);

private slots:
    // The frame timer handler.
    void onTimer();

    // UI handlers.
    void on_continueButton_clicked();
    void on_nextButton_clicked();
    void on_stopButton_clicked();
    void on_moveButton_clicked();
    void on_openScriptFileButton_clicked();
    void on_writeButton_clicked();
    void on_errorButton_clicked();
    void on_scriptView_textChanged();

    void on_actionExport_for_Windows_triggered();
    void on_actionExport_for_macOS_triggered();
    void on_actionExport_for_iOS_triggered();
    void on_actionExport_for_Android_triggered();
    void on_actionExport_for_Web_triggered();
    void on_actionExport_package_only();

private:
    // The rendering timer.
    QTimer *m_timer;

    // The sound devices.
    //QIODevice *m_soundDevice[MIXER_STREAMS];

    // Whether we are in English mode.
    bool m_isEnglish;

    // Get the cursor line.
    int getCursorLine();

    // Update the script model from text view content.
    void updateScriptModelFromText();

    // File copy helper.
    void copyPath(QString src, QString dst);

public:
    // For Qt Creator. (contains UI objects such as ui->continueButton)
    Ui::MainWindow *ui;

    //
    // Note:
    //  the following members are currently public to be used from the HAL
    //  functions that are declared in platform.h and implemented in mainwindow.cpp
    //

    // The unique instance of this class.
    static MainWindow *obj;

    // State.
    bool m_isRunning;
    bool m_isResumePressed;
    bool m_isNextPressed;
    bool m_isPausePressed;
    bool m_isOpenScriptPressed;
    bool m_isExecLineChanged;
    int m_changedExecLine;

    // View update.
    void setWaitingState();
    void setRunningState();
    void setStoppedState();
    void updateScriptView();
    void updateVariableText();
    void scrollScript();

#ifdef USE_QT_AUDIO
    // The sound sinks.
    QAudioSink *m_soundSink[MIXER_STREAMS];

    // Wave streams.
    struct wave *m_wave[MIXER_STREAMS];

    // Whether waves stream are finished.
    bool m_waveFinish[MIXER_STREAMS];
#endif
};

#endif // MAINWINDOW_H
