#include "Graphics.h"

using namespace std;


float Graphics::ratioSize = 2.0f;

Graphics::Graphics(Memory *m, sf::RenderWindow *app)
{
	_memory = m;
	_app = app;

	_address = 0;
	_codeRegister = 0;
	_mode = 0;
	_controlByte = 0;

	for(int i = 0 ; i < GRAPHIC_VRAM_SIZE ; i++)
		_vram[i] = 0;

	for(int i = 0 ; i < GRAPHIC_REGISTER_SIZE ; i++)
		_register[i] = 0;

   if(!font.loadFromFile("data/LiberationSans-Regular.ttf"))
   {
      cerr << "Cannot open the font !" << endl;
   }

   timeCycle = 0;

	//_drawImage.loadFromFile("lena.jpg");
}

void Graphics::draw()
{
   sf::Text text;
   text.setFont(font);

   text.setCharacterSize(24);
   text.setColor(sf::Color::Blue);
   sf::Time elapsed = clock.restart();
   timeCycle = timeCycle*90.f/100.f + 10.f/100.f*(1000000.0f/(float)elapsed.asMicroseconds());
   std::ostringstream ss;
   int *most = Stats::getMost();
   ss << timeCycle << endl << "mode: " << (uint16_t)_mode << endl << "most used : " << endl << "\t" << getOpcodeName(most[0]) << " - " << Stats::opcodeOccur[most[0]] << endl << "\t" << getOpcodeName(most[1]) << " - " << Stats::opcodeOccur[most[1]] << endl << "\t" << getOpcodeName(most[2]) << " - " << Stats::opcodeOccur[most[2]] << endl << "\t" << getOpcodeName(most[3]) << " - " << Stats::opcodeOccur[most[3]] << endl << "\t" << getOpcodeName(most[4]) << " - " << Stats::opcodeOccur[most[4]] << endl << "\t" << getOpcodeName(most[5]) << " - " << Stats::opcodeOccur[most[5]] << endl << "\t" << getOpcodeName(most[6]) << " - " << Stats::opcodeOccur[most[6]] << endl << "\t" << getOpcodeName(most[7]) << " - " << Stats::opcodeOccur[most[7]] << endl << "\t" << getOpcodeName(most[8]) << " - " << Stats::opcodeOccur[most[8]];
   text.setString(ss.str());

	/*loadDrawImage();

	_tex.loadFromImage(_drawImage);
	sf::Sprite sp(_tex);

	_app->draw(sp);*/
	_app->draw(text);
}

uint8_t Graphics::read(uint8_t port)
{
	if(port == 0x7E)
		return 0;
	else if(port == 0x7F)
		return 0;
}

uint8_t Graphics::write(uint8_t port, uint8_t data)
{
	if(port == 0xBE) // data port
	{
		if(_codeRegister == 3)
			_cram[_address] = data;
		else // code == 0,1,2
			_vram[_address] = data;
		_address++;
		_address %= 0x4000;
	}
	else if(port == 0xBF) // control port
	{
		if(_controlByte == 0)
		{
			_address = (_address & 0xFF00) + data;
			_controlCmd = data;
			_controlByte++;
		}
		else
		{
			_address = (_address & 0xFF) + ((data&(~(0b11<<6)))<<8);
			_controlCmd += (data << 8);
			_controlByte = 0;

			slog << ldebug << "Graphic control new address:" << hex << _address << endl;

			controlAction();
		}
	}
}

uint8_t Graphics::controlAction()
{
	uint8_t _codeRegister = (_controlCmd >> 14);

	slog << ldebug << hex << "Graphic control code:" << (uint16_t)_codeRegister << endl;

	if(_codeRegister == 0)
	{
		/// TODO manage read
	}
	else if(_codeRegister == 1)
	{

	}
	else if(_codeRegister == 2)
	{
		uint8_t num = (_controlCmd&(0xF<<8));
		_register[num] = _controlCmd&0xFF;

		slog << ldebug << hex << "Graphic mode:" << (uint16_t)(_register[0]&0b100==0b100) << (uint16_t)(_register[1]&0b1000==0b1000) << (uint16_t)(_register[0]&0b1==0b1) << (uint16_t)(_register[1]&0b10000==0b10000) << endl;
	}
	else if(_codeRegister == 3)
	{

	}

	if(!getBit8(_register[0], 1) && !getBit8(_register[1],3) && getBit8(_register[1],4))
		_mode = 1;
	else if(getBit8(_register[0], 1) && !getBit8(_register[1],3) && !getBit8(_register[1],4))
		_mode = 2;
	else if(!getBit8(_register[0], 1) && getBit8(_register[1],3) && !getBit8(_register[1],4))
		_mode = 3;
}


// Private :

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
