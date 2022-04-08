#include "ui/MainWindow.h"
#include "ui_MainWindow.h"

#include <thread>

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

#if USE_QT_6
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

#include "Breakpoint.h"
#include "CPU.h"
#include "Cartridge.h"
#include "Debugger.h"
#include "Graphics.h"
#include "Inputs.h"
#include "Log.h"
#include "System.h"
#include "ui/GameWindow.h"
#include "ui/SettingsInputsWindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), _settings { "Emul-IX", "Emul-IX" }, _systemRunning { false }
{
	ui->setupUi(this);

	setWindowTitle("EmulIX MasterSystem");
	loadGameLibrary();

	_system = std::make_unique<System>();

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

void MainWindow::quit() { stopSystemThread(); }

void MainWindow::slot_systemThreadTerminated()
{
	if(_gameWindow) {
		_gameWindow->close();
	}

	ui->listWidget_gameLibrary->setEnabled(true);
	ui->pushButton_play->setEnabled(true);
	ui->pushButton_stop->setEnabled(false);

	//    _system->getCartridge().ptr()->remove();
	_system = std::make_unique<System>();
}

void MainWindow::on_actionAbout_qt_triggered() { QMessageBox::aboutQt(this, "About Qt"); }

void MainWindow::on_actionAbout_triggered() { QMessageBox::about(this, "About", "No info here yet!"); }

void MainWindow::on_actionPlay_triggered() { }

void MainWindow::on_actionQuit_triggered() { quit(); }

void MainWindow::on_actionSettings_triggered()
{
	std::unique_ptr<SettingsInputsWindow> w = std::make_unique<SettingsInputsWindow>(this);
	w->exec();
}

void MainWindow::on_listWidget_gameLibrary_currentRowChanged(int iCurrentRow)
{
	if(iCurrentRow != -1) {
		ui->pushButton_play->setEnabled(true);
	}
}

void MainWindow::on_listWidget_gameLibrary_itemDoubleClicked(QListWidgetItem* item)
{
	if(!_system->getCartridge().ptr()->isLoaded()) {
		_system->getCartridge().ptr()->insert(item->text().toStdString());
	}
	startSystemThread();
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
			ui->listWidget_gameLibrary->setCurrentRow(ui->listWidget_gameLibrary->count() - 1);
			saveGameLibrary();
		}
	} catch(EmulatorException& exception) {
		SLOG(lwarning << exception.what());
	}
}

void MainWindow::on_pushButton_play_clicked()
{
	if(ui->listWidget_gameLibrary->currentRow() == -1) {
		return;
	}
	if(!_system->getCartridge().ptr()->isLoaded()) {
		_system->getCartridge().ptr()->insert(ui->listWidget_gameLibrary->currentItem()->text().toStdString());
	}
	startSystemThread();
}

void MainWindow::on_pushButton_quit_clicked()
{
	quit();
	close();
}

void MainWindow::on_pushButton_stop_clicked() { stopSystemThread(); }

// Private:

void MainWindow::loadGameLibrary()
{
	QList<QVariant> gameList = _settings.value("gui/gameLibrary").toList();
	ui->listWidget_gameLibrary->clear();
	for(int i = 0; i < gameList.count(); i++) {
		ui->listWidget_gameLibrary->addItem(gameList[i].toString());
	}
}

void MainWindow::saveGameLibrary()
{
	QList<QVariant> listGames;
	for(int i = 0; i < ui->listWidget_gameLibrary->count(); i++) {
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
		Inputs::Instance()->aknowledgeStopRequest();
		_gameWindow = std::make_unique<GameWindow>();
		_gameWindow->setResolution(GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
		_gameWindow->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, _gameWindow->sizeHint(),
#if USE_QT_6
			qApp->primaryScreen()->availableGeometry()
#else
			qApp->desktop()->availableGeometry()
#endif
				));
		_gameWindow->show();
		_system->getGraphics().ptr()->setGameWindow(_gameWindow.get());
		_systemThread = std::thread(&MainWindow::systemThread, this);
	}
}

void MainWindow::stopSystemThread()
{
	_systemRunning = false;
	if(_systemThread.joinable()) {
		_systemThread.join();
	}
}

void MainWindow::systemThread()
{
	using namespace std::literals::chrono_literals;

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

	Inputs* inputs = Inputs::Instance();
	Graphics* g = _system->getGraphics().ptr();
	CPU* cpu = _system->getCpu().ptr();

	Debugger* debugger = Debugger::Instance();
	// Example of breakpoints:
	// debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kNumInstruction, 1234));
	// debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kAddress, 0x4321));

	g->startRunning();
	cpu->init();

	double offsetTiming = 0.0;

	std::chrono::time_point<std::chrono::steady_clock> chronoSync = std::chrono::steady_clock::now();
	std::chrono::duration<long double, std::micro> intervalCycle;
	while(_systemRunning && !inputs->isStopRequested()) {
		int nbTStates = 0;

		if(debugger->manage(cpu->getProgramCounter()) == Debugger::State::kRunning) {
			nbTStates = cpu->cycle();
			debugger->addNumberCycle(nbTStates);
		} else {
			Log::typeMin = Log::ALL;
			Log::printConsole = true;
			// Log::printFile = true;
			std::this_thread::sleep_for(1ms);
		}

		const long double expectedExecTime = nbTStates * CPU::MicrosecondPerState;

		intervalCycle = std::chrono::steady_clock::now() - chronoSync;
		Stats::addExecutionStat(nbTStates, intervalCycle.count());

		while(intervalCycle.count() < expectedExecTime) {
			intervalCycle = std::chrono::steady_clock::now() - chronoSync;
		}

		chronoSync = std::chrono::steady_clock::now();
	}

	g->stopRunning();

	_systemRunning = false;
	emit(signal_systemThreadTerminated());
}
