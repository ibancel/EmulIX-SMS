#pragma once

#include <thread>

#include <QMainWindow>
#include <QSettings>

#include "System.h"
#include "ui/GameWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void quit();

signals:
    void signal_systemThreadTerminated();

private slots:
    void slot_systemThreadTerminated();
    void on_actionAbout_qt_triggered();
    void on_actionAbout_triggered();
    void on_actionPlay_triggered();
    void on_actionQuit_triggered();
    void on_listWidget_gameLibrary_currentRowChanged(int currentRow);
    void on_pushButton_browseBios_clicked();
    void on_pushButton_clearLibrary_clicked();
    void on_pushButton_loadFile_clicked();
    void on_pushButton_play_clicked();
    void on_pushButton_quit_clicked();
    void on_pushButton_stop_clicked();


private:
    Ui::MainWindow *ui;
    QSettings _settings;
    std::unique_ptr<System> _system;
    bool _systemRunning;
    std::unique_ptr<GameWindow> _gameWindow;
    std::thread _systemThread;

    void loadGameLibrary();
    void saveGameLibrary();
    void startSystemThread();
    void stopSystemThread();
    void systemThread();
};
