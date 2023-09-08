#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Rendering timer handler.
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

private:
    // The rendering timer.
    QTimer *m_timer;

    // Whether we are in English mode.
    bool m_isEnglish;

public:
    // For Qt Creator. (contains UI objects such as ui->continueButton)
    Ui::MainWindow *ui;

    // The unique instance of this class.
    static MainWindow *obj;

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
};

#endif // MAINWINDOW_H
