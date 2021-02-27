#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "definitions.h"
#include "ui/GameWindow.h"

class GraphicsThread
{
public:
	static constexpr int ImageWidth = 256;
	static constexpr int ImageHeight = 192;

	std::mutex mutexSync;
	std::condition_variable conditionSync;

	GraphicsThread();
	~GraphicsThread();

    static PixelColor nibbleToPixelColor(std::uint_fast8_t nibble);
    static PixelColor byteToPixelColor(uint_fast8_t iByte, bool useTransparency = false);
	
	// Thread control
	void close(bool iWaitJoin);
	void reset();
	void start();

	// Display
	void drawInfo();
	void drawGame();

	// Actions
	void resetLineInterrupt();

	// Gets:
	u16 getCounterV();
	u8 getRegister(u8 iIndex);
	u8 getStatusRegister();
	u8 getStatusRegisterBit(u8 position);
	u8 getVram(u16 iAddress);

	// Sets:
	void setCram(u16 iAddress, u8 iNewValue);
	void setGraphicMode(u8 iNewMode);
	void setRegister(u8 iIndex, u8 iNewValue);
	void setStatusRegister(u8 iNewValue);
	void setStatusRegisterBit(u8 position, bool newBit);
	void setVram(u16 iAddress, u8 iNewValue);
    void setGameWindow(GameWindow* iWindow);


	bool isSynchronized() {
		return _isSynchronized;
	}

	void addCpuStates(int iNumberStates) {
		std::scoped_lock lock{ _mutexData };
		_cpuStatesExecuted += iNumberStates;
		setIsSynchronized(false);
	}

	bool getIE() {
		bool frameInterrupt = static_cast<bool>(getStatusRegisterBit(VDP::S_F)) && static_cast<bool>(getBit8(_register[1], 5));
		bool lineInterrupt = _lineInterruptFlag && static_cast<bool>(getBit8(_register[0], 4));
		return frameInterrupt || lineInterrupt;
	}

private:
	std::thread _threadGame;
	std::thread _threadInfo;
	bool _isRunning;

    GameWindow* _gameWindow;

	u8 _graphicMode;
	u8 _register[GRAPHIC_REGISTER_SIZE];
	u8 _statusRegister;
	u8 _tempLineRegister;
	u8 _vram[GRAPHIC_VRAM_SIZE];
	u8 _cram[GRAPHIC_CRAM_SIZE];

    Frame _frame;

	long double _cpuStatesExecuted;

	u16 _hCounter;
	u16 _vCounter;
	u8 _lineInterruptCounter;
	bool _lineInterruptFlag;

	bool _isSynchronized;

	std::chrono::time_point<std::chrono::steady_clock> _chronoDispInfo;
	std::chrono::time_point<std::chrono::steady_clock> _chronoDispGame;
	std::chrono::time_point<std::chrono::steady_clock> _pixelTimer;

	std::mutex _mutexData;

	int _frameCounter;
	float _framePerSecond;

	// To remove:
	int _runningBarState;

	void drawFrame();
	void drawLine(int line);
	void drawPatterns();
	void drawPalettes();
    void getFrame() { /* TODO */ }
	void runThreadGame();
	void runThreadInfo();

	void setIsSynchronized(bool iState) {
		std::lock_guard<std::mutex> aLock{ this->mutexSync };
		_isSynchronized = iState;
		this->conditionSync.notify_all();
	}

	// NAME TABLE
	inline u16 getNameTableAddress() {
		// TODO: different for other resolutions
		return (_register[2] & 0x0E) << 10;
	}
    u8* getNameTable(u16 memoryOffset = 0);

	// COLOR TABLE
	inline u16 getColorTableAddress() {
		return _register[3] << 6;
	}
    u8* getColorTable(u8 memoryOffset = 0);

	inline u16 getColorTableAddressMode2() {
		return (_register[3] & 0b1000'0000) << 6;
	}
    u8* getColorTableMode2(u16 memoryOffset = 0);

	// PATTERN GENERATOR TABLE
	inline u16 getPatternGeneratorAddress() {
		return (_register[4] & 0b111) * 0x800;
	}
    u8* getPatternGenerator(u16 memoryOffset = 0);

	inline u16 getPatternGeneratorAddressMode2() {
		return (_register[4] & 0b100) * 0x800;
	}
    u8* getPatternGeneratorMode2(u16 memoryOffset = 0);

	inline u16 getPatternGeneratorAddressMode4() {
		return 0;
	}
    u8* getPatternGeneratorMode4(u16 memoryOffset = 0);

	// SPRITE ATTRIBUTE TABLE
	inline u16 getSpriteAttributeTableAddress() {
		//return (_register[5] & 0b1111111) * 0x80;
		return (_register[5] & 0x7E) << 7;
	}
    u8* getSpriteAttributeTable(u8 memoryOffset = 0);

	// SPRITE PATTERN TABLE
	// Also called Sprite Generator Table
	inline u16 getSpritePatternTableAddress() {
		return (_register[6] & 0b100) << 11;
	}
    u8* getSpritePatternTable(u16 memoryOffset = 0);



	inline u8 getBackdropColor() const {
		return (_register[7] & 0xF);
	}

	inline u8 getTextColor() const {
		return (_register[7] >> 4);
	}

	inline bool isActiveDisplay() const {
		return getBit8(_register[1], 6);
	}

	inline SpriteSize getSpriteSize() const {
		return static_cast<SpriteSize>(getBit8(_register[0], 1));
	}
};
