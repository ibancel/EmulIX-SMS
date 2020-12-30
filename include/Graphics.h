#pragma once

#include <chrono>

#include "GraphicsThread.h"
#include "Memory.h"
#include "Log.h"
#include "Stats.h"
#include "System.h"
#include "types.h"

class Memory;
class System;

class Graphics
{
public:
	//static constexpr long double PixelFrequency = 5'376'240.0 * TIME_SCALE; // NTSC
	//static constexpr double PixelFrequency = 5'352'300.0 * TIME_SCALE; // PAL
	static constexpr long double PixelFrequency = BaseFrequency / 2.0;
	static float RatioSize;

    Graphics(System& parent);

	void reset();

	void startRunning();
	void stopRunning();
	void syncThread();

	u8 read(const u8 port);
	u8 write(const u8 port, const u8 data);

	u8 controlAction();

    void setGameWindow(GameWindow* iWindow);

	void dumpVram();

	u8 getGraphicMode();

	bool getIE() {
		return _graphicsThread.getIE();
	}

	inline void addCpuStates(int iNumberStates) {
		_graphicsThread.addCpuStates(iNumberStates);
	}

#if !GRAPHIC_THREADING
	void drawGame();
	void drawInfo();
#endif

protected:
    PtrRef<Memory> _memory;

	GraphicsThread _graphicsThread;

	u16 _addressVRAM;
	u8 _codeRegister;
	u8 _actualMode;

	u8 _controlByte;
	u16 _controlCmd;
	u8 _readAheadBuffer;

	// to remove
	int _count;
	bool _debugDrawSprite;

	void updateGraphicMode();

	inline void incrementVramAddress() {
		_addressVRAM++;
		if (_addressVRAM >= 0x4000) {
			_addressVRAM = 0;
		}
	}
};
