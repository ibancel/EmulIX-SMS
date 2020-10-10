#include "ui/MainWindow.h"
#include "ui_MainWindow.h"

#include <thread>

#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>

#include "Breakpoint.h"
#include "Cartridge.h"
#include "CPU.h"
#include "Debugger.h"
#include "Graphics.h"
#include "Inputs.h"
#include "Log.h"
#include "ui/GameWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      _settings{ "Emul-IX", "Emul-IX"},
      _systemRunning{ false }
{
    ui->setupUi(this);

    setWindowTitle("EmulIX MasterSystem");
    loadGameLibrary();

    connect(this, &MainWindow::signal_systemThreadTerminated, this, &MainWindow::slot_systemThreadTerminated);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(_systemThread.joinable()) {
        _systemRunning = false;
        _systemThread.join();
    }
}

void MainWindow::quit()
{
    stopSystemThread();
    close();
}

void MainWindow::slot_systemThreadTerminated()
{
    stopSystemThread();
}

void MainWindow::on_actionAbout_qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "No info here yet!");
}

void MainWindow::on_actionPlay_triggered()
{

}

void MainWindow::on_actionQuit_triggered()
{
    quit();
}


void MainWindow::on_listWidget_gameLibrary_currentRowChanged(int iCurrentRow)
{
    if(iCurrentRow != -1) {
        ui->pushButton_play->setEnabled(true);
    }
}

void MainWindow::on_pushButton_browseBios_clicked()
{
    QString biosFolder = _settings.value("gui/biosFolder", "").toString();
    QString filenameBios = QFileDialog::getOpenFileName(this, "Choose the bios");
    if(filenameBios != "") {
        _settings.setValue("gui/biosFolder", QFileInfo(filenameBios).absoluteDir().absolutePath());
    }
}

void MainWindow::on_pushButton_clearLibrary_clicked()
{
    ui->listWidget_gameLibrary->clear();
    saveGameLibrary();
}

void MainWindow::on_pushButton_loadFile_clicked()
{
    QString romFolder = _settings.value("gui/romFolder", "").toString();
    QString filenameGame = QFileDialog::getOpenFileName(this, "Open a ROM", romFolder);
    try {
        if(filenameGame != "") {
            _settings.setValue("gui/romFolder", QFileInfo(filenameGame).absoluteDir().absolutePath());
            ui->listWidget_gameLibrary->addItem(QFileInfo(filenameGame).absoluteFilePath());
            ui->listWidget_gameLibrary->setCurrentRow(ui->listWidget_gameLibrary->count()-1);
            saveGameLibrary();
        }
    } catch(EmulatorException& exception) {
        SLOG(lwarning << exception.what());
    }
}

void MainWindow::on_pushButton_play_clicked()
{
    if(ui->listWidget_gameLibrary->currentRow() == -1 || Cartridge::Instance()->isLoaded()) {
        return;
    }
    Cartridge::Instance()->insert(ui->listWidget_gameLibrary->currentItem()->text().toStdString());
    startSystemThread();
}

void MainWindow::on_pushButton_quit_clicked()
{
    quit();
}

void MainWindow::on_pushButton_stop_clicked()
{
    stopSystemThread();
    Cartridge::Instance()->remove();
}

// Private:

void MainWindow::loadGameLibrary()
{
    QList<QVariant> gameList = _settings.value("gui/gameLibrary").toList();
    ui->listWidget_gameLibrary->clear();
    for(int i = 0 ; i < gameList.count() ; i++) {
        ui->listWidget_gameLibrary->addItem(gameList[i].toString());
    }
}

void MainWindow::saveGameLibrary()
{
    QList<QVariant> listGames;
    for(int i = 0 ; i < ui->listWidget_gameLibrary->count() ; i++) {
        listGames.push_back(ui->listWidget_gameLibrary->item(i)->text());
    }
    _settings.setValue("gui/gameLibrary", listGames);
}

void MainWindow::startSystemThread()
{
    if(!_systemRunning) {
        ui->listWidget_gameLibrary->setEnabled(false);
        ui->pushButton_play->setEnabled(false);
        ui->pushButton_stop->setEnabled(true);

        if(_systemThread.joinable()) {
            _systemThread.join();
        }
        _systemRunning = true;
        _gameWindow = std::make_unique<GameWindow>();
        _gameWindow->setResolution(GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
        _gameWindow->setGeometry(
            QStyle::alignedRect(
                   Qt::LeftToRight,
                   Qt::AlignCenter,
                   _gameWindow->sizeHint(),
                   qApp->desktop()->availableGeometry()
               )
        );
        _gameWindow->show();
        Graphics::Instance()->setGameWindow(_gameWindow.get());
        _systemThread = std::thread(&MainWindow::systemThread, this);
    }
}

void MainWindow::stopSystemThread()
{
    _systemRunning = false;
    if(_systemThread.joinable()) {
        _systemThread.join();
    }

    if(_gameWindow) {
        _gameWindow->close();
    }

    ui->listWidget_gameLibrary->setEnabled(true);
    ui->pushButton_play->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
}

void MainWindow::systemThread()
{
    using namespace std;

    Graphics::RatioSize = 1.0f;

    // Example: Log::typeMin = Log::ALL & (~Log::DEBUG);
#if DEBUG_MODE
    Log::printConsole = false;
    Log::printFile = true;
    Log::typeMin = Log::ALL;
#else
    Log::typeMin = Log::NONE;
#endif

    Log::exitOnWarning = true;

    Inputs *inputs = Inputs::Instance();
    Cartridge *cartridge = Cartridge::Instance();
    Memory *mem = Memory::Instance();
    Graphics *g = Graphics::Instance();

    CPU *cpu = CPU::Instance();

    Debugger* debugger = Debugger::Instance();
    // Example of breakpoints:
    //debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kNumInstruction, 1234));
    //debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kAddress, 0x4321));

    cpu->init();

    double offsetTiming = 0.0;

    std::chrono::time_point<std::chrono::steady_clock> chronoSync = chrono::steady_clock::now();
    chrono::duration<long double, std::micro> intervalCycle;
    while (_systemRunning && !inputs->isStopRequested())
    {
        int nbTStates = 0;

        if (debugger->manage(cpu->getProgramCounter()) == Debugger::State::kRunning) {
            nbTStates = cpu->cycle();
            debugger->addNumberCycle(nbTStates);
        } else {
            Log::typeMin = Log::ALL;
            Log::printConsole = true;
            //Log::printFile = true;
            std::this_thread::sleep_for(1ms);
        }

        const long double expectedExecTime = nbTStates * CPU::MicrosecondPerState;

        intervalCycle = chrono::steady_clock::now() - chronoSync;
        Stats::addExecutionStat(nbTStates, intervalCycle.count());

        while (intervalCycle.count() < expectedExecTime) {
            intervalCycle = chrono::steady_clock::now() - chronoSync;
        }

        chronoSync = chrono::steady_clock::now();
    }

    g->stopRunning();

    emit(signal_systemThreadTerminated());
    _systemRunning = false;
}
