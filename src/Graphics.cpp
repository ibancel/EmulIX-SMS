#include "Graphics.h"

#include <iomanip>

using namespace std;


float Graphics::RatioSize = 2.0f;


Graphics::Graphics() : Graphics{Memory::Instance(), nullptr}
{

}

Graphics::Graphics(Memory *m, sf::RenderWindow *winInfo)
{
	_memory = Memory::Instance();
	_winInfo = nullptr;
	_winGame = nullptr;

	_addressVRAM = 0;
	_codeRegister = 0;
	_mode = 0;
	_controlByte = 0;
	_controlCmd = 0;
	_readAheadBuffer = 0;

	for (int i = 0; i < GRAPHIC_VRAM_SIZE; i++)
		_vram[i] = 0;

	for (int i = 0; i < GRAPHIC_REGISTER_SIZE; i++)
		_register[i] = 0;

	//_register[3] = 0x2C; // CRAM sarting address?

	_statusRegister = 0;
	_tempLineRegister = 0;
	_lineInterruptCounter = 0;
	_lineInterruptFlag = false;

	if (!font.loadFromFile("data/LiberationSans-Regular.ttf"))
	{
		cerr << "Cannot open the font !" << endl;
	}

	timeCycle = 0;

	_cpuStatesExecuted = 0;

	_runningBarState = 0;
	_infoPrintMode = 0;

	_chronoDispInfo = chrono::steady_clock::now() - 1s;
	_chronoDispGame = chrono::steady_clock::now() - 1s;
	_pixelTimer = chrono::steady_clock::now();

	reset();
}

void Graphics::reset()
{
	setBit8(&_statusRegister, S_F, 0);
	_pixelTimer = chrono::steady_clock::now() - 1s;
	_register[0x0] = 0;
	_register[0x1] = 0;
	_register[0x2] = 0xFF;
	_register[0x3] = 0xFF;
	_register[0x5] = 0xFF;
	_register[0x4] = 0x07;
	_register[0xA] = 0xFF;
	_tempLineRegister = 0;
	// should verify if the following instructions works well:
	_count = 0;
	_vCounter = 0;
	_hCounter = 0;
	_lineInterruptCounter = 0;
	_lineInterruptFlag = false;
	_readAheadBuffer = 0;
}

void Graphics::drawInfo()
{
	if (!_winInfo) {
		throw "No window found for drawing";
	}

	sf::Time elapsed = clock.restart();
	timeCycle = timeCycle * 98.5 / 100.0 + 1.5 / 100.0 * (1000000.0 / elapsed.asMicroseconds());

	chrono::duration<double, std::milli> intervalDisplay = chrono::steady_clock::now() - _chronoDispInfo;
	if (intervalDisplay.count() < 30) { 
		return;
	}
	_chronoDispInfo = chrono::steady_clock::now();

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	sf::Text text;
	text.setFont(font);

	text.setCharacterSize(22);
	text.setFillColor(sf::Color::Blue);

	std::ostringstream ss;

	if (_infoPrintMode == 0) {
		int* most = Stats::getMost();
		ss << timeCycle << endl << "mode: " << static_cast<int>(_mode) << endl << "most used : " << endl << "\t" << getOpcodeName(0, most[0]) << " - " << Stats::opcodeOccur[most[0]] << endl << "\t" << getOpcodeName(0, most[1]) << " - " << Stats::opcodeOccur[most[1]] << endl << "\t" << getOpcodeName(0, most[2]) << " - " << Stats::opcodeOccur[most[2]] << endl << "\t" << getOpcodeName(0, most[3]) << " - " << Stats::opcodeOccur[most[3]] << endl << "\t" << getOpcodeName(0, most[4]) << " - " << Stats::opcodeOccur[most[4]] << endl << "\t" << getOpcodeName(0, most[5]) << " - " << Stats::opcodeOccur[most[5]] << endl << "\t" << getOpcodeName(0, most[6]) << " - " << Stats::opcodeOccur[most[6]] << endl << "\t" << getOpcodeName(0, most[7]) << " - " << Stats::opcodeOccur[most[7]] << endl << "\t" << getOpcodeName(0, most[8]) << " - " << Stats::opcodeOccur[most[8]];
		text.setString(ss.str());
	} else if (_infoPrintMode == 1) {
		ss << timeCycle << endl << "mode: " << static_cast<int>(_mode) << endl << "CRAM at "
			<< hex << std::setfill('0')
			<< std::setw(4) << getColorTableAddress() << ": " << endl
			<< endl
			<< "VRAM pointer: " << std::setw(4) << _addressVRAM;
		text.setString(ss.str());
	}

	sf::Text textReg;
	textReg.setFont(font);
	textReg.setCharacterSize(22);
	textReg.setFillColor(sf::Color::Blue);
	textReg.setPosition(_winInfo->getSize().x - 150, 0);
	std::ostringstream ssReg;
	ssReg << "Status: " << hex << uint16_t(_statusRegister) << endl << endl;
	for(int i = 0 ; i < GRAPHIC_REGISTER_SIZE ; i++)
		ssReg << "#" << i << ": " << hex << uint16_t(_register[i]) << endl;
	textReg.setString(ssReg.str());

	sf::Text textFlag;
	textFlag.setFont(font);
	textFlag.setCharacterSize(22);
	textFlag.setFillColor(sf::Color::Blue);
	textFlag.setPosition(10, _winInfo->getSize().y - 30);
	std::ostringstream ssFlag;
	uint8_t r = CPU::Instance()->getRegisterFlag();
	ssFlag << hex << "Flag: S=" << ((r >> 7) & 1) << " Z=" << ((r >> 6) & 1);
	ssFlag << hex << " N=" << ((r >> 4) & 1) << " P/V=" << ((r >> 2) & 1);
	ssFlag << hex << " N=" << ((r >> 1) & 1) << " C=" << ((r >> 0) & 1);
	textFlag.setString(ssFlag.str());


	sf::Text textDebugger;
	textDebugger.setFont(font);
	textDebugger.setCharacterSize(22);
	textDebugger.setFillColor(sf::Color::Blue);
	textDebugger.setPosition(10, _winInfo->getSize().y - 60);
	if (Debugger::Instance()->getState() == Debugger::State::kRunning) {
		if (_runningBarState == 0) {
			textDebugger.setString(" |");
			_runningBarState++;
		} else if (_runningBarState == 1) {
			textDebugger.setString(" /");
			_runningBarState++;
		} else if(_runningBarState == 2) {
			textDebugger.setString("---");
			_runningBarState++;
		} else {
			textDebugger.setString(" \\");
			_runningBarState = 0;
		}
	} else {
		textDebugger.setFillColor(sf::Color::Red);
		textDebugger.setString("||");
	}

	/*for(int i = 0 ; i < _winInfo->getSize().x ; i++) {
		for(int j = 0 ; j < _winInfo->getSiz)
	}*/

	//_winInfo->resetGLStates();
	/*loadDrawImage();

	_tex.loadFromImage(_drawImage);
	sf::Sprite sp(_tex);

	_winInfo->draw(sp);*/

	_winInfo->clear(sf::Color::Black);
	_winInfo->draw(text);
	_winInfo->draw(textReg);
	_winInfo->draw(textFlag);
	_winInfo->draw(textDebugger);
	_winInfo->display();
}

void Graphics::drawGame()
{
#if GRAPHIC_PRECISE_TIMING
	double decimalPixelNumber = _cpuStatesExecuted * (Graphics::PixelFrequency / CPU::BaseFrequency);
	int nbPixel = static_cast<int>(decimalPixelNumber);
	_cpuStatesExecuted = (decimalPixelNumber - nbPixel) / (Graphics::PixelFrequency / CPU::BaseFrequency);
# else
	chrono::duration<double, std::micro> intervalPixelTimer = chrono::steady_clock::now() - _pixelTimer;
	int nbPixel = intervalPixelTimer.count() / ((1.0 / 5.37624) / TIME_SCALE);
#endif

	for (; nbPixel > 0; --nbPixel) {
	//if (intervalPixelTimer.count() >= (1.0f/15720.0f/TIME_SCALE)) { // TODO PAL/NTSC
		//cout << dec << intervalPixelTimer.count() << " | " << (uint16_t)_hCounter << " | " << (uint16_t)_vCounter << endl;
		_hCounter++;
		if (_hCounter >= 342) {
			_hCounter = 0;
			_vCounter++;
			if (_vCounter <= 192) {
				_lineInterruptCounter--;
				if (_lineInterruptCounter >= 0xFF) {
					_lineInterruptCounter = _tempLineRegister;
					_lineInterruptFlag = true;
				}
			} else {
				_tempLineRegister = _register[0xA];
				_lineInterruptCounter = _tempLineRegister;
			}
			if (_vCounter == 192) {
				setBit8(&_statusRegister, S_F, 1);
			}
			if (_vCounter >= 262) {
				_vCounter = 0;
			}
		}
		_pixelTimer = chrono::steady_clock::now();
	}

	chrono::duration<double, std::milli> intervalDisplay = chrono::steady_clock::now() - _chronoDispGame;
	if (intervalDisplay.count() < 16) {
		return;
	}
	_chronoDispGame = chrono::steady_clock::now();
	uint8_t backdropColor = getColorTable(ColorBank::kFirst)[getBackdropColor()];
	_winGame->clear(byteToSfmlColor(backdropColor));
	if (isActiveDisplay()) {
		if(_infoPrintMode == 0) {
			loadDrawImage();
			_winGame->draw(sf::Sprite(_tex));
		} else if (_infoPrintMode == 1) {
			drawPatterns();
		}
	}
	drawPalettes();

    _winGame->display();
}

uint8_t Graphics::read(const uint8_t port)
{
	if (port == 0x7E) { // V counter
		return (_vCounter > 0xDA ? _vCounter - 6 : _vCounter);
		// TODO other values NTSC/PAL
		return 0;
	}  else if (port == 0x7F) { // H counter
		NOT_IMPLEMENTED("READ H COUNTER");
		return 0;
	} else if (port == 0xBE) { // data port
		_controlByte = 0;
		// TODO verify
		uint8_t result = _readAheadBuffer;
		_readAheadBuffer = _vram[_addressVRAM];
		incrementVramAddress();
		return result;
	}  else if (port == 0xBF) { // control port
		_controlByte = 0;
		uint8_t retStatusRegister = _statusRegister;
		setBit8(&_statusRegister, S_F, 0);
		setBit8(&_statusRegister, S_C, 0);
		_lineInterruptFlag = false;
		//NOT_IMPLEMENTED("READ CONTROL PORT");
		return retStatusRegister; // TODO handle OVR (overflow) & COL (collision)
	} else {
		SLOG_THROW(lwarning << hex << "Undefined Graphic port (" << static_cast<uint16_t>(port) << ") ");
	}

	return 0;
}

uint8_t Graphics::write(const uint8_t port, const uint8_t data)
{
	if(port == 0xBE) // data port
	{
		//std::cout << hex << "[VDP] pc=" << CPU::instance()->getProgramCounter() << " | write data=" << (uint16_t)data << " at " << _addressVRAM << " & coderegister=" << (uint16_t)_codeRegister << endl;
		SLOG(ldebug << hex << "[VDP] write data=" << (uint16_t)data << " at " << _addressVRAM << " & coderegister=" << (uint16_t)_codeRegister << endl);
		if (_codeRegister == 3) {
			*getColorTable(_addressVRAM & 0b11111) = data;
		} else { // code == 0,1,2
			_vram[_addressVRAM] = data;
		}
		incrementVramAddress();		
		_readAheadBuffer = data;
		_controlByte = 0;
	}
	else if(port == 0xBF) // control port
	{
		if(_controlByte == 0)
		{
			_controlCmd = data;
			_controlByte++;
		}
		else
		{
			_controlCmd += (data << 8);
			_controlByte = 0;

			controlAction();
		}
	} else {
		slog << lwarning << hex << "Undefined Graphic port (" << static_cast<uint16_t>(port) << ") " << endl;
	}

	return 0; // TODO
}

uint8_t Graphics::controlAction()
{
	_codeRegister = (_controlCmd >> 14);

	SLOG(ldebug << hex << "[VPD] control code:" << (uint16_t)_codeRegister << endl);

	if (_codeRegister != 2) {
		_addressVRAM = _controlCmd & 0b00111111'11111111;
		SLOG(ldebug << "[VDP] control new address: " << hex << _addressVRAM << endl);
	}

	//std::cout << hex << "[VDP] pc=" << CPU::instance()->getProgramCounter() << " | control code=" << (uint16_t)_codeRegister << " new address=" << _addressVRAM << endl;

	if (_codeRegister == 0) {
		// Read VRAM
		_readAheadBuffer = _vram[_addressVRAM];
		incrementVramAddress();
	} else if (_codeRegister == 1) {
		// Write VRAM, address is set: nothing more
	} else if (_codeRegister == 2) {
		uint8_t num = ((_controlCmd & 0xF00) >> 8);
		if (num < GRAPHIC_REGISTER_SIZE) {
			_register[num] = _controlCmd & 0xFF;
		}

		SLOG(ldebug << hex << "[VDP] set r[" << (uint16_t)num << "] = " << (uint16_t)(_controlCmd & 0xFF));
		SLOG(ldebug << hex << "[VDP] mode:" << (uint16_t)getBit8(_register[0], 2) << (uint16_t)getBit8(_register[1], 3) << (uint16_t)getBit8(_register[0], 1) << (uint16_t)getBit8(_register[1], 4));
	} else if (_codeRegister == 3) {
		// Read CRAM, address is set: nothing more
	} else {
		NOT_IMPLEMENTED("UNDEFINED CODE REGISTER");
	}

#if MACHINE_VERSION == 1
	const bool mode1 = getBit8(_register[1], 4);
	const bool mode2 = getBit8(_register[0], 1);
	const bool mode3 = getBit8(_register[1], 3);
	const bool mode4 = getBit8(_register[0], 2);
	if (mode4) {
		if (!mode1 || mode2) {
			_mode = 4;
		} else {
			_mode = 1;
		}
	} else {
		if (!mode3 && !mode2 && !mode1) {
			_mode = 0;
		} else if (!mode3 && !mode2 && mode1) {
			_mode = 1;
		} else if (!mode3 && mode2 && !mode1) {
			_mode = 2;
		} else if (mode3 && !mode2 && !mode1) {
			_mode = 3;
		} else {
			NOT_IMPLEMENTED("DOUBLE VIDEO MODE");
		}
	}
#else
	NOT_IMPLEMENTED("Machine version");
#endif

	return 0; // TODO
}

void Graphics::setWindowInfo(sf::RenderWindow *win)
{
	_winInfo = win;
}

void Graphics::setWindowGame(sf::RenderWindow *win)
{
	_winGame = win;
}

void Graphics::dumpVram()
{
	if (DumpMode == 0) {
		std::ofstream file("vdp_dump.txt", std::ios_base::out | std::ios_base::binary);
		for (int i = 0; i < GRAPHIC_VRAM_SIZE; i++) {
			file << _vram[i];
		}
		file.close();
	} else if (DumpMode == 1) {
		std::ofstream file("vdp_dump.txt", std::ios_base::out);
		for (int i = 0; i < GRAPHIC_VRAM_SIZE; i++) {
			file << std::hex << std::setfill('0') << std::uppercase << std::setw(2) << (uint16_t)(_vram[i]) << " ";
			if (i != 0 && (i % 16) == 15) {
				file << std::endl;
			}
		}
	}
}

// Private :

sf::Color Graphics::nibbleToSfmlColor(std::uint_fast8_t nibble)
{
	constexpr uint32_t colorArray[]{
		0x00000000,
		0x000000FF,
		0x21C842FF,
		0x5EDC78FF,
		0x5455EDFF,
		0x7D76FCFF,
		0xD4524DFF,
		0x42EBF5FF,
		0xFC5554FF,
		0xFF7978FF,
		0xD4C154FF,
		0xE6CE80FF,
		0x21B03BFF,
		0xC95BBAFF,
		0xCCCCCCFF,
		0xFFFFFFFF
	};

	return sf::Color(colorArray[static_cast<size_t>(nibble & 0xF)]);
}

sf::Color Graphics::byteToSfmlColor(std::uint_fast8_t iByte, bool useTransparency)
{
	if (useTransparency && (iByte&0b111111) == 0) {
		return sf::Color(0, 0, 0, 0);
	}
	return sf::Color( (iByte&0b11)*85, ((iByte >> 2)&0b11)*85,  ((iByte >> 4)&0b11)*85 );
}

uint8_t* const Graphics::getNameTable(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getNameTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const Graphics::getColorTable(uint8_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getColorTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const Graphics::getColorTableMode2(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getColorTableAddressMode2()) + static_cast<uintptr_t>(memoryOffset);
}

// PATTERN GENERATOR TABLE
uint8_t* const Graphics::getPatternGenerator(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getPatternGeneratorAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const Graphics::getPatternGeneratorMode2(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getPatternGeneratorAddressMode2()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const Graphics::getPatternGeneratorMode4(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getPatternGeneratorAddressMode4()) + static_cast<uintptr_t>(memoryOffset);
}

//SPRITE ATTRIBUTE TABLE
uint8_t* const Graphics::getSpriteAttributeTable(uint8_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getSpriteAttributeTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const Graphics::getSpritePatternTable(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getSpritePatternTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}

void Graphics::loadDrawImage()
{
	int width = 256;
	int height = 192;

	_drawImage.create(width, height, sf::Color(255, 255, 255));
	_tex.create(width, height);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const uint_fast8_t characterY = y / 8;
			const uint8_t rowPatternIndex = (y - (static_cast<uint16_t>(characterY) << 3)) << 2; // "<<" for faster multiplication
			if (_mode == 0)
			{
				const uint_fast8_t characterX = x / 8;
				const uint_fast16_t characterPosition = characterX + characterY * 32;
				uint8_t patternBaseAddress = getNameTable()[characterPosition];

				uint8_t bitPixel = getBit8(getPatternGenerator(8 * patternBaseAddress)[y - characterY], 7 - (x - characterX));
				uint_fast8_t colorByte = (getColorTable())[patternBaseAddress / 8];
				//_drawImage.setPixel(x, y, byteToSfmlColor((bitPixel ? colorByte : 16 + colorByte)));
				_drawImage.setPixel(x, y, nibbleToSfmlColor((bitPixel ? colorByte >> 4 : colorByte & 0xF)));
			} else if (_mode == 2)
			{
				const uint_fast8_t characterX = x / 8;
				const uint_fast16_t characterPosition = characterX + characterY * 32;
				uint16_t patternBaseAddress = getNameTable()[characterPosition];

				if (characterY >= 8 && characterY < 16) {
					patternBaseAddress += 0x100;
				} else if (characterY >= 16) {
					patternBaseAddress += 0x200;
				}

				patternBaseAddress *= 8;

				uint8_t bitPixel = getBit8(getPatternGeneratorMode2(patternBaseAddress)[y - characterY], 7 - (x - characterX));
				uint_fast8_t colorByte = (getColorTableMode2(patternBaseAddress))[y - characterY];
				_drawImage.setPixel(x, y, nibbleToSfmlColor((bitPixel ? colorByte >> 4 : colorByte & 0xF)));
			} else if (_mode == 4)
			{
				const uint_fast8_t characterX = x / 8;
				const uint_fast16_t characterPosition = characterX + characterY * 32;
				uint_fast16_t wordName = (getNameTable(characterPosition * 2)[1] << 8) | getNameTable(characterPosition * 2)[0];
				uint16_t patternIndex = (wordName & 0x1FF) * 32; // "<<5" = "*32"

				wordName >>= 9; // Care about using it in the following lines
				uint_fast8_t flipH = wordName & 1;
				wordName >>= 1;
				uint_fast8_t flipV = wordName & 1;
				wordName >>= 1;
				uint_fast8_t paletteSelect = wordName & 1;
				wordName >>= 1;
				uint_fast8_t priority = wordName & 1;

				uint8_t const* const pattern = getPatternGeneratorMode4(patternIndex);
				const uint8_t columnPatternIndex = 7 - (x - (static_cast<uint16_t>(characterX) << 3));
				uint8_t selectedColor = getBit8(pattern[rowPatternIndex], columnPatternIndex);
				selectedColor |= getBit8(pattern[rowPatternIndex + 1], columnPatternIndex) << 1;
				selectedColor |= getBit8(pattern[rowPatternIndex + 2], columnPatternIndex) << 2;
				selectedColor |= getBit8(pattern[rowPatternIndex + 3], columnPatternIndex) << 3;

				uint_fast8_t colorByte = getColorTable(paletteSelect << 4)[selectedColor];
				_drawImage.setPixel(x, y, byteToSfmlColor(colorByte, true));
			} 
			else
			{
				//uint_fast8_t patternPosX = x / 8;
				//uint_fast8_t patternPosY = y / 8;
				//uint_fast8_t nameTableAddress = _register[2] & 0xF;
				//uint_fast16_t patternPosition = patternPosX + patternPosY * 32;
				//uint_fast16_t wordName = _vram[nameTableAddress + patternPosition];

				//uint_fast16_t patternIndex = wordName & 0b111111111;
				//wordName >>= 9; // Care about using it in the following lines
				//uint_fast8_t flipH = wordName & 1;
				//wordName >>= 1;
				//uint_fast8_t flipV = wordName & 1;
				//wordName >>= 1;
				//uint_fast8_t paletteSelect = wordName & 1;
				//wordName >>= 1;
				//uint_fast8_t priority = wordName & 1;

				//uint_fast8_t bitPixel = _vram[8 * patternIndex + y - patternPosY] >> (x - patternPosX);
				//uint_fast8_t colorByte = (getColorTable())[patternBaseAddress / 8];
				//_drawImage.setPixel(x, y, byteToSfmlColor(bitPixel ? colorByte : 16 + colorByte));
				NOT_IMPLEMENTED("Unknown mode");
			}
		}
	}

	if (false) // Sprite subsystem
	{
		// TODO: rewrite this for mode 4 (maybe still good for mode 2?)
		uint8_t const* const spriteAttributeTable = getSpriteAttributeTable();

		for (uint_fast8_t indexSprite = 0; indexSprite < 32; indexSprite++) {
			const int8_t y = spriteAttributeTable[indexSprite * 4 + 0];
			const int8_t x = spriteAttributeTable[indexSprite * 4 + 1];
			const uint8_t name = spriteAttributeTable[indexSprite * 4 + 2];
			const uint8_t tag = spriteAttributeTable[indexSprite * 4 + 3]; // color + EC (early clock)
			const uint8_t colorSprite = tag & 0xF;

			const uint8_t xOffset = getBit8(tag, 7) * 32;

			const uint8_t spriteSize = getSpriteSize() == SPRITE_SIZE::k16 ? 16 : 8;

			if (spriteSize == 8) {
				for (uint8_t yPixel = 0; yPixel < spriteSize; yPixel++) {
					const int16_t realY = yPixel + y + 1;
					if (realY < 0 || realY > GRAPHIC_HEIGHT) {
						continue;
					}
					for (uint8_t xPixel = 0; xPixel < spriteSize; xPixel++) {
						const int16_t realX = xPixel + x + xOffset;
						if (realX < 0 || realX > GRAPHIC_WIDTH) {
							continue;
						}
						const uint8_t pixelState = getBit8(getSpriteAttributeTable(name * 8)[yPixel], xPixel);
						_drawImage.setPixel(realX, realY, nibbleToSfmlColor(pixelState ? colorSprite : 0));
					}
				}
			} else if (spriteSize == 16) {
				for (uint8_t yPixel = 0; yPixel < spriteSize; yPixel++) {
					const int16_t realY = yPixel + y + 1;
					if (realY < 0 || realY > GRAPHIC_HEIGHT) {
						continue;
					}
					for (uint8_t xPixel = 0; xPixel < spriteSize; xPixel++) {
						const int16_t realX = xPixel + x + xOffset;
						if (realX < 0 || realX > GRAPHIC_WIDTH) {
							continue;
						}
						const uint8_t pixelPatternOffsetY = xPixel >= 8 ? 16 : 0;
						const uint8_t pixelState = getBit8(getSpriteAttributeTable(name * 32)[yPixel + pixelPatternOffsetY], xPixel % 8);
						_drawImage.setPixel(realX, realY, nibbleToSfmlColor(pixelState ? colorSprite : 0));
						//_drawImage.setPixel(realX, realY, sf::Color::White);
					}
				}
			} else {
				NOT_IMPLEMENTED("UNKNOWN SPRITE SIZE");
			}
		}
	}
	_tex.loadFromImage(_drawImage);
}

void Graphics::drawPatterns()
{
	int width = 256;
	int height = 128;

	sf::Image aImg;
	aImg.create(width, height, sf::Color(0, 0, 0));
	sf::Texture aTex;
	aTex.create(width, height);

	for (int y = 0; y < height; y++)
	{
		const uint8_t characterY = y / 8;
		const uint8_t rowPatternIndex = (y - (static_cast<uint16_t>(characterY) * 8)) << 2; // "<<" for faster "*"
		for (int x = 0; x < width; x++)
		{
			const uint8_t characterX = x / 8;
			uint16_t patternIndex = characterX + characterY * 32;

			patternIndex <<= 5; // = patternIndex * 32

			uint8_t const* const pattern = &_vram[patternIndex];
			
			const uint8_t columnPatternIndex = 7 - (x - (static_cast<uint16_t>(characterX) * 8));
			uint8_t selectedColor = getBit8(pattern[rowPatternIndex], columnPatternIndex);
			selectedColor |= getBit8(pattern[rowPatternIndex + 1], columnPatternIndex) << 1;
			selectedColor |= getBit8(pattern[rowPatternIndex + 2], columnPatternIndex) << 2;
			selectedColor |= getBit8(pattern[rowPatternIndex + 3], columnPatternIndex) << 3;

			uint8_t colorByte = getColorTable(ColorBank::kFirst)[selectedColor];
			aImg.setPixel(x, y, byteToSfmlColor(colorByte, true));
		}
	}
	aTex.loadFromImage(aImg);
	sf::Sprite aSprite(aTex);
	aSprite.setPosition(0, 0);
	_winGame->draw(aSprite);
}

void Graphics::drawPalettes()
{
	int width = 256;
	int height = 192;

	sf::Image aImg;
	aImg.create(256, 32, sf::Color(255, 0, 0));
	sf::Texture aTex;
	aTex.create(256, 32);

	for (uint_fast16_t y = 0; y < 32; y++)
	{
		for (uint_fast16_t x = 0; x < 256; x++)
		{
			uint_fast8_t patternPosX = x / 16;
			uint_fast8_t patternPosY = y / 16;
			uint_fast8_t colorByte = getColorTable(patternPosY*16)[patternPosX];

			if (x % 16 == 0 /*|| (x + 1) % 16 == 0*/ || x == 255 || y % 16 == 0 || y == 31/*|| (y + 1) % 16 == 0*/) {
				aImg.setPixel(x, y, sf::Color::White);
			} else {
				aImg.setPixel(x, y, byteToSfmlColor(colorByte));
			}
		}
	}
	aTex.loadFromImage(aImg);
	sf::Sprite aSprite(aTex);
	aSprite.setPosition(0, 200);
	_winGame->draw(aSprite);
}