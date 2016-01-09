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

	void read(uint8_t port, uint16_t data);
	void write(uint8_t port, uint16_t data);

private:
	Memory *_memory;
	sf::RenderWindow *_app;
	sf::Image _drawImage;
	sf::Texture _tex;

	void loadDrawImage();

};

#endif
