#pragma once

#include <thread>

#include <QObject>

#include "CPU.h"
#include "Cartridge.h"
#include "Graphics.h"
#include "Memory.h"
#include "types.h"

class Cartridge;
class CPU;
class Graphics;
class Memory;

class System : public QObject
{
	Q_OBJECT
public:
	System();
	virtual ~System();

	PtrRef<Cartridge> getCartridge();
	PtrRef<CPU> getCpu();
	PtrRef<Graphics> getGraphics();
	PtrRef<Memory> getMemory();

	// u8 portCommunication(bool iWrite, u8 address, u8 data = 0);

	void start();
	void shutdown();

signals:
	void signal_systemTerminated();

private:
	Cartridge* _cartridge;
	CPU* _cpu;
	Graphics* _graphics;
	bool _isRunning;
	Memory* _memory;
	std::thread _thread;

	void internalRun();
};
