#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioSink>
#include <QIODevice>
#include <QTimer>

extern "C" {
#include "suika.h"
#include "package.h"
#include "glrender.h"
};

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
    void on_fileNameEdit_returnPressed();
    void on_lineNumberEdit_returnPressed();
    void on_updateScriptFileButton_clicked();
    void on_openScriptFileButton_clicked();
    void on_updateCommandButton_clicked();
    void on_resetCommandButton_clicked();
    void on_writeButton_clicked();
    void on_reloadButton_clicked();
    void on_overwriteButton_clicked();
    void on_errorButton_clicked();
    void on_updateLineNumberButton_clicked();
    void on_scriptListView_doubleClicked(const QModelIndex &index);

    void on_actionExport_data01_arc_triggered();

    void on_actionExport_for_Web_triggered();

private:
    // The rendering timer.
    QTimer *m_timer;

    // The sound devices.
    QIODevice *m_soundDevice[MIXER_STREAMS];

    // Whether we are in English mode.
    bool m_isEnglish;

public:
    // For Qt Creator. (contains UI objects such as ui->continueButton)
    Ui::MainWindow *ui;

    //
    // Note:
    //  the following members are currently public to be used from the HAL
    //  and the debugger-HAL functions that are declared in platform.h and
    //  implemented in mainwindow.cpp
    //

    // The unique instance of this class.
    static MainWindow *obj;

    // The sound sinks.
    QAudioSink *m_soundSink[MIXER_STREAMS];

    // Wave streams.
    struct wave *m_wave[MIXER_STREAMS];

    // Whether waves stream are finished.
    bool m_waveFinish[MIXER_STREAMS];

    // State. (read/written from
    bool m_isRunning;
    bool m_isResumePressed;
    bool m_isNextPressed;
    bool m_isPausePressed;
    bool m_isChangeScriptPressed;
    bool m_isChangeLinePressed;
    bool m_isCommandUpdatePressed;
    bool m_isReloadPressed;

    // View update.
    void setWaitingState();
    void setRunningState();
    void setStoppedState();
    void updateScriptView();
    void updateVariableText();
    void scrollScript();
};

#endif // MAINWINDOW_H
