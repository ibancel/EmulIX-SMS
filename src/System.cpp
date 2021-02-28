#include "System.h"

#include <thread>

#include <QObject>

#include "CPU.h"
#include "Cartridge.h"
#include "Graphics.h"
#include "Memory.h"

System::System() : _isRunning { false }
{
	_cartridge = new Cartridge(*this);
	_cpu = new CPU(*this);
	_graphics = new Graphics(*this);
	_memory = new Memory(*this);
}

System::~System()
{
	delete _cartridge;
	delete _cpu;
	delete _graphics;
	delete _memory;
}

PtrRef<Cartridge> System::getCartridge() { return PtrRef<Cartridge> { _cartridge }; }

PtrRef<CPU> System::getCpu() { return PtrRef<CPU> { _cpu }; }

PtrRef<Graphics> System::getGraphics() { return PtrRef<Graphics> { _graphics }; }

PtrRef<Memory> System::getMemory() { return PtrRef<Memory> { _memory }; }

void System::start()
{
	_isRunning = true;
	_thread = std::thread(&System::internalRun, this);
}

void System::shutdown()
{
	_isRunning = false;
	if(_thread.joinable()) {
		_thread.join();
	}
}

// Private:

void System::internalRun()
{
	while(_isRunning) { }
	emit(signal_systemTerminated());
}
