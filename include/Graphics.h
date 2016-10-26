#ifndef _H_EMULATOR_GRAPHICS
#define _H_EMULATOR_GRAPHICS

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Memory.h"
#include "Log.h"
#include "Stats.h"
#include "CPU.h"

class CPU;

enum STATUS_BITS { S_F = 7, S_5S = 6, S_C = 5 };

class Graphics : public Singleton<Graphics>
{

public:
	static float ratioSize;

	Graphics();
	Graphics(Memory *m, sf::RenderWindow *app);

	void reset();

	void draw();

	uint8_t read(uint8_t port);
	uint8_t write(uint8_t port, uint8_t data);

	uint8_t controlAction();

	void setWindow(sf::RenderWindow *win);

	sf::RenderWindow* getWindow() const { return _app; }

	uint8_t getIE() {
		uint8_t val = getBit8(_statusRegister, S_F);
		_statusRegister = setBit8(_statusRegister, S_F, 0);
		return val;
	}

private:
	Memory *_memory;
	sf::RenderWindow *_app;
	sf::Image _drawImage;
	sf::Texture _tex;

	uint8_t _vram[GRAPHIC_VRAM_SIZE];
	uint8_t _cram[GRAPHIC_VRAM_SIZE];

	uint8_t _statusRegister;
	uint8_t _register[GRAPHIC_REGISTER_SIZE];

	uint16_t _addressVRAM;
	uint8_t _codeRegister;
	uint8_t _mode;

	uint8_t _controlByte;
	uint16_t _controlCmd;

	// to remove
	int _count;

	// SFML
	sf::Font font;
	sf::Clock clock;
	int timeCycle;


	void loadDrawImage();

};

#endif
