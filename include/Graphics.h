#ifndef _H_EMULATOR_GRAPHICS
#define _H_EMULATOR_GRAPHICS

#include <chrono>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Memory.h"
#include "Log.h"
#include "Stats.h"
#include "CPU.h"

class CPU;

enum STATUS_BITS { S_F = 7, S_5S = 6, S_C = 5 };

enum class SPRITE_SIZE : uint8_t { k8=0, k16=1 };

enum ColorBank { kFirst = 0, kSecond = 16 };

class Graphics : public Singleton<Graphics>
{

public:
	static constexpr int DumpMode = 0;
	static constexpr double PixelFrequency = 5'376'240.0;
	static float RatioSize;

	Graphics();
	Graphics(Memory *m, sf::RenderWindow *winInfo);

	void reset();

	void drawInfo();
	void drawGame();

	uint8_t read(const uint8_t port);
	uint8_t write(const uint8_t port, const uint8_t data);

	uint8_t controlAction();

	void setWindowInfo(sf::RenderWindow *win);
	void setWindowGame(sf::RenderWindow *win);

	void dumpVram();

	sf::RenderWindow* getWindowInfo() const { return _winInfo; }
	sf::RenderWindow* getWindowGame() const { return _winGame; }

	bool getIE() {
		//return false;
		bool frameInterrupt = static_cast<bool>(getBit8(_statusRegister, S_F)) && static_cast<bool>(getBit8(_register[1], 5));
		bool lineInterrupt = _lineInterruptFlag && static_cast<bool>(getBit8(_register[0], 4));
		return frameInterrupt || lineInterrupt;
	}

	inline int getInfoPrintMode() const {
		return _infoPrintMode;
	}

	inline void setInfoPrintMode(const int iMode) {
		_infoPrintMode = iMode;
	}

	inline void switchInfoPrintMode() {
		_infoPrintMode++;
		if (_infoPrintMode > 1) {
			_infoPrintMode = 0;
		}
	}

	inline void addCpuStates(int iNumberStates) {
		_cpuStatesExecuted += iNumberStates;
	}

private:
	Memory *_memory;
	sf::RenderWindow *_winInfo;
	sf::RenderWindow *_winGame;
	sf::Image _drawImage;
	sf::Texture _tex;

	uint8_t _vram[GRAPHIC_VRAM_SIZE];
	//uint8_t _cram[GRAPHIC_CRAM_SIZE];

	uint8_t _statusRegister;
	uint8_t _register[GRAPHIC_REGISTER_SIZE];

	uint16_t _addressVRAM;
	uint8_t _codeRegister;
	uint8_t _mode;

	uint8_t _controlByte;
	uint16_t _controlCmd;
	uint8_t _readAheadBuffer;

	uint8_t _tempLineRegister;
	uint16_t _hCounter;
	uint16_t _vCounter;
	uint8_t _lineInterruptCounter;
	bool _lineInterruptFlag;

	double _cpuStatesExecuted;

	std::chrono::time_point<std::chrono::steady_clock> _pixelTimer;

	// to remove
	int _count;
	int _runningBarState;
	int _infoPrintMode;

	std::chrono::time_point<std::chrono::steady_clock> _chronoDispInfo;
	std::chrono::time_point<std::chrono::steady_clock> _chronoDispGame;


	// SFML
	sf::Font font;
	sf::Clock clock;
	int timeCycle;

	static sf::Color nibbleToSfmlColor(std::uint_fast8_t nibble);

	static sf::Color byteToSfmlColor(uint_fast8_t iByte, bool useTransparency = false);

	inline void incrementVramAddress() {
		_addressVRAM++;
		if (_addressVRAM >= 0x4000) {
			_addressVRAM = 0;
		}
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
	uint8_t * const getColorTable(uint8_t memoryOffset = 0);

	inline uint16_t getColorTableAddressMode2() {
		return (_register[3] & 0b1000'0000) << 6;
	}
	uint8_t* const getColorTableMode2(uint16_t memoryOffset = 0);

	// PATTERN GENERATOR TABLE
	inline uint16_t getPatternGeneratorAddress() {
		return (_register[4]&0b111) * 0x800;
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
		//return (_register[6]&0b111) * 0x800;
		return (_register[6] & 0b100) << 11;
	}
	uint8_t* const getSpritePatternTable(uint16_t memoryOffset = 0);

	void loadDrawImage();
	void drawPatterns();
	void drawPalettes();



	inline uint8_t getBackdropColor() const {
		return (_register[7] & 0xF);
	}

	inline uint8_t getTextColor() const {
		return (_register[7] >> 4);
	}

	inline bool isActiveDisplay() const {
		return getBit8(_register[1], 6);
	}

	inline SPRITE_SIZE getSpriteSize() const {
		return static_cast<SPRITE_SIZE>(getBit8(_register[0], 1));
	}
};

#endif
