#pragma once

#include <thread>
#include <mutex>

#include <SFML/Graphics.hpp>

#include "definitions.h"

class GraphicsThread
{
public:
	static constexpr int ImageWidth = 256;
	static constexpr int ImageHeight = 192;

	std::mutex mutexSync;
	std::condition_variable conditionSync;

	GraphicsThread();
	~GraphicsThread();

	static sf::Color nibbleToSfmlColor(std::uint_fast8_t nibble);
	static sf::Color byteToSfmlColor(uint_fast8_t iByte, bool useTransparency = false);
	
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
	uint16_t getCounterV();
	uint8_t getRegister(uint8_t iIndex);
	uint8_t getStatusRegister();
	uint8_t getStatusRegisterBit(uint8_t position);
	uint8_t getVram(uint16_t iAddress);
	sf::RenderWindow* getWindowInfo() const { return _winInfo; }
	sf::RenderWindow* getWindowGame() const { return _winGame; }

	// Sets:
	void setCram(uint16_t iAddress, uint8_t iNewValue);
	void setGraphicMode(uint8_t iNewMode);
	void setRegister(uint8_t iIndex, uint8_t iNewValue);
	void setStatusRegister(uint8_t iNewValue);
	void setStatusRegisterBit(uint8_t position, bool newBit);
	void setVram(uint16_t iAddress, uint8_t iNewValue);
	void setWindowInfo(sf::RenderWindow* win);
	void setWindowGame(sf::RenderWindow* win);


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

	sf::RenderWindow* _winGame;
	sf::RenderWindow* _winInfo;

	bool _requestWinInfoActivation;
	bool _requestWinGameActivation;

	uint8_t _graphicMode;
	uint8_t _register[GRAPHIC_REGISTER_SIZE];
	uint8_t _statusRegister;
	uint8_t _tempLineRegister;
	uint8_t _vram[GRAPHIC_VRAM_SIZE];
	uint8_t _cram[GRAPHIC_CRAM_SIZE];

	long double _cpuStatesExecuted;

	uint16_t _hCounter;
	uint16_t _vCounter;
	uint8_t _lineInterruptCounter;
	bool _lineInterruptFlag;

	bool _isSynchronized;

	std::chrono::time_point<std::chrono::steady_clock> _chronoDispInfo;
	std::chrono::time_point<std::chrono::steady_clock> _chronoDispGame;
	std::chrono::time_point<std::chrono::steady_clock> _pixelTimer;

	std::mutex _mutexData;

	int _frameCounter;
	float _framePerSecond;

	// SFML
	sf::Font font;
	sf::Clock _clockFPS;
	sf::Image _drawImage;
	sf::Texture _tex;

	// To remove:
	int _runningBarState;

	void drawFrame();
	void drawLine(int line);
	void drawPatterns();
	void drawPalettes();
	void runThreadGame();
	void runThreadInfo();

	void setIsSynchronized(bool iState) {
		std::lock_guard<std::mutex> aLock{ this->mutexSync };
		_isSynchronized = iState;
		this->conditionSync.notify_all();
	}

	// NAME TABLE
	inline uint16_t getNameTableAddress() {
		// TODO: different for other resolutions
		return (_register[2] & 0x0E) << 10;
	}
	uint8_t* const getNameTable(uint16_t memoryOffset = 0);

	// COLOR TABLE
	inline uint16_t getColorTableAddress() {
		return _register[3] << 6;
	}
	uint8_t* const getColorTable(uint8_t memoryOffset = 0);

	inline uint16_t getColorTableAddressMode2() {
		return (_register[3] & 0b1000'0000) << 6;
	}
	uint8_t* const getColorTableMode2(uint16_t memoryOffset = 0);

	// PATTERN GENERATOR TABLE
	inline uint16_t getPatternGeneratorAddress() {
		return (_register[4] & 0b111) * 0x800;
	}
	uint8_t* const getPatternGenerator(uint16_t memoryOffset = 0);

	inline uint16_t getPatternGeneratorAddressMode2() {
		return (_register[4] & 0b100) * 0x800;
	}
	uint8_t* const getPatternGeneratorMode2(uint16_t memoryOffset = 0);

	inline uint16_t getPatternGeneratorAddressMode4() {
		return 0;
	}
	uint8_t* const getPatternGeneratorMode4(uint16_t memoryOffset = 0);

	// SPRITE ATTRIBUTE TABLE
	inline uint16_t getSpriteAttributeTableAddress() {
		//return (_register[5] & 0b1111111) * 0x80;
		return (_register[5] & 0x7E) << 7;
	}
	uint8_t* const getSpriteAttributeTable(uint8_t memoryOffset = 0);

	// SPRITE PATTERN TABLE
	// Also called Sprite Generator Table
	inline uint16_t getSpritePatternTableAddress() {
		return (_register[6] & 0b100) << 11;
	}
	uint8_t* const getSpritePatternTable(uint16_t memoryOffset = 0);



	inline uint8_t getBackdropColor() const {
		return (_register[7] & 0xF);
	}

	inline uint8_t getTextColor() const {
		return (_register[7] >> 4);
	}

	inline bool isActiveDisplay() const {
		return getBit8(_register[1], 6);
	}

	inline SpriteSize getSpriteSize() const {
		return static_cast<SpriteSize>(getBit8(_register[0], 1));
	}
};