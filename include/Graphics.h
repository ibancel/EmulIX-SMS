#ifndef _H_EMULATOR_GRAPHICS
#define _H_EMULATOR_GRAPHICS

#include <chrono>

#include "GraphicsThread.h"
#include "Memory.h"
#include "Log.h"
#include "Stats.h"
#include "CPU.h"

class CPU;

class Graphics : public Singleton<Graphics>
{
public:
	//static constexpr long double PixelFrequency = 5'376'240.0 * TIME_SCALE; // NTSC
	//static constexpr double PixelFrequency = 5'352'300.0 * TIME_SCALE; // PAL
	static constexpr long double PixelFrequency = BaseFrequency / 2.0;
	static float RatioSize;

	Graphics();

	void reset();

	void startRunning();
	void stopRunning();
	void syncThread();

	uint8_t read(const uint8_t port);
	uint8_t write(const uint8_t port, const uint8_t data);

	uint8_t controlAction();

    void setGameWindow(GameWindow* iWindow);

	void dumpVram();

	uint8_t getGraphicMode();

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
	Memory *_memory;

	GraphicsThread _graphicsThread;

	uint16_t _addressVRAM;
	uint8_t _codeRegister;
	uint8_t _actualMode;

	uint8_t _controlByte;
	uint16_t _controlCmd;
	uint8_t _readAheadBuffer;

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

#endif
