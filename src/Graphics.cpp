#include "Graphics.h"

float Graphics::ratioSize = 2.f;

Graphics::Graphics(Memory *m, sf::RenderWindow *app)
{
	_memory = m;
	_app = app;

	//_drawImage.loadFromFile("lena.jpg");
}

void Graphics::draw()
{
	loadDrawImage();

	_tex.loadFromImage(_drawImage);
	sf::Sprite sp(_tex);
	_app->draw(sp);
}

void Graphics::loadDrawImage()
{
	unsigned int width = _app->getSize().x;
	unsigned int height = _app->getSize().y;

	_drawImage.create(width, height, sf::Color(255,255,255));
	_tex.create(width, height);

	for(int i = 0 ; i < width ; i++)
	{
		for(int j = 0 ; j < height ; j++)
		{
			_drawImage.setPixel(i,j, sf::Color(255,0,0));
		}
	}
	//_drawImage.loadFromMemory(pixels, 8*GRAPHIC_WIDTH*GRAPHIC_HEIGHT*4);
}
