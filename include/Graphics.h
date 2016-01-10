#ifndef _H_EMULATOR_GRAPHICS
#define _H_EMULATOR_GRAPHICS

#include <SFML/Graphics.hpp>

#include "Memory.h"
#include "Log.h"

class Graphics
{

public:
	static float ratioSize;

	Graphics(Memory *m, sf::RenderWindow *app);

	void draw();

	uint8_t read(uint8_t port);
	uint8_t write(uint8_t port, uint8_t data);

	uint8_t controlAction();

private:
	Memory *_memory;
	sf::RenderWindow *_app;
	sf::Image _drawImage;
	sf::Texture _tex;

	uint8_t _vram[GRAPHIC_VRAM_SIZE];
	uint8_t _cram[GRAPHIC_VRAM_SIZE];

	uint8_t _register[GRAPHIC_REGISTER_SIZE];

	uint16_t _address;
	uint8_t _codeRegister;
	uint8_t _mode;

	uint8_t _controlByte;
	uint16_t _controlCmd;


	void loadDrawImage();

};

#endif
