#include "GraphicsThread.h"

#include <chrono>
#include <cmath>
#include <iomanip>

#include <SFML/Graphics.hpp>

#include "definitions.h"
#include "Frame.h"
#include "Graphics.h"


using namespace std::literals;

GraphicsThread::GraphicsThread() : 
    _gameWindow{ nullptr },
	_vram{ 0 },
	_cram{ 0 },
	_register{ 0 },
    _frame{ImageWidth, ImageHeight, PixelColor{0,255,0,255}},
	_graphicMode{ 0 },
	_isSynchronized{ true },
	_frameCounter{ 0 },
	_framePerSecond{ 0 }
{
//	if (!font.loadFromFile("data/LiberationSans-Regular.ttf"))
//	{
//		std::cerr << "Cannot open the font !" << std::endl;
//	}

	_cpuStatesExecuted = 0;
	_statusRegister = 0;
	_tempLineRegister = 0;

	_runningBarState = 0;
}

GraphicsThread::~GraphicsThread()
{

}

PixelColor GraphicsThread::nibbleToPixelColor(std::uint_fast8_t nibble)
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

    return PixelColor(colorArray[static_cast<size_t>(nibble & 0xF)]);
}

PixelColor GraphicsThread::byteToPixelColor(std::uint_fast8_t iByte, bool useTransparency)
{
    if (useTransparency && ((iByte & 0b111111) == 0)) {
        return PixelColor{0, 0, 0, 0};
    }
    return PixelColor{
        static_cast<uint8_t>((iByte & 0b11) * 85),
        static_cast<uint8_t>(((iByte >> 2) & 0b11) * 85),
        static_cast<uint8_t>(((iByte >> 4) & 0b11) * 85),
        255
    };
}

void GraphicsThread::close(bool iWaitJoin)
{
	if (_isRunning) {
		_isRunning = false;
		if (iWaitJoin) {
			if (_threadGame.joinable()) {
				_threadGame.join();
			}
			if (_threadInfo.joinable()) {
				_threadInfo.join();
			}
		}
	}
}

void GraphicsThread::reset()
{
	_pixelTimer = std::chrono::steady_clock::now() - 100ms;
	_chronoDispInfo = std::chrono::steady_clock::now() - 100ms;
	_chronoDispGame = std::chrono::steady_clock::now() - 100ms;

	setBit8(&_statusRegister, VDP::S_F, 0);
	_register[0x0] = 0;
	_register[0x1] = 0;
	_register[0x2] = 0xFF;
	_register[0x3] = 0xFF;
	_register[0x5] = 0xFF;
	_register[0x4] = 0x07;
	_register[0xA] = 0xFF;
	_tempLineRegister = 0;

	_graphicMode = 0;

	// Unsure
	_tempLineRegister = 0;
	_vCounter = 0;
	_hCounter = 0;
	_lineInterruptCounter = 0;
	_lineInterruptFlag = false;

	setIsSynchronized(true);

    _frame = Frame(ImageWidth, ImageHeight, PixelColor{255,0,0,255});
}

void GraphicsThread::start()
{
	#if GRAPHIC_THREADING
	if (!_isRunning) {
		_isRunning = true;
		_threadGame = std::thread(&GraphicsThread::runThreadGame, this);
	}
	#endif
}

void GraphicsThread::drawInfo()
{
//	if (!_winInfo) {
//		throw "No window found for drawing";
//	}

//	if (_clockFPS.getElapsedTime().asSeconds() >= 1.0f) {
//		_clockFPS.restart();
//		_framePerSecond = _frameCounter;
//		_frameCounter = 0;
//	}

//	std::chrono::duration<double, std::milli> intervalDisplay = std::chrono::steady_clock::now() - _chronoDispInfo;
//	if (intervalDisplay.count() < 16) {
//		return;
//	}
//	_chronoDispInfo = std::chrono::steady_clock::now();

//	float time = _clockFPS.getElapsedTime().asSeconds();
//	int actualFPS = round(_frameCounter + ((1.0f - time) * _framePerSecond));

//	sf::Text text;
//	text.setFont(font);

//	text.setCharacterSize(22);
//	text.setFillColor(sf::Color::Blue);

//	std::ostringstream ss;

//	int infoDispMode = Inputs::Instance()->getInfoDisplayMode();
//	if (infoDispMode == 0) {
//		int* most = Stats::getMost();
//		ss << actualFPS << " (" << (int)(Stats::getCpuExecSpeed()*100.0) << "%)" << std::endl
//			<< "mode: " << static_cast<int>(_graphicMode) << std::endl
//			<< "most used : " << std::endl
//			<< "\t" << getOpcodeName(0, most[0]) << " - " << Stats::opcodeOccur[most[0]] << std::endl
//			<< "\t" << getOpcodeName(0, most[1]) << " - " << Stats::opcodeOccur[most[1]] << std::endl
//			<< "\t" << getOpcodeName(0, most[2]) << " - " << Stats::opcodeOccur[most[2]] << std::endl
//			<< "\t" << getOpcodeName(0, most[3]) << " - " << Stats::opcodeOccur[most[3]] << std::endl
//			<< "\t" << getOpcodeName(0, most[4]) << " - " << Stats::opcodeOccur[most[4]] << std::endl
//			<< "\t" << getOpcodeName(0, most[5]) << " - " << Stats::opcodeOccur[most[5]] << std::endl
//			<< "\t" << getOpcodeName(0, most[6]) << " - " << Stats::opcodeOccur[most[6]] << std::endl
//			<< "\t" << getOpcodeName(0, most[7]) << " - " << Stats::opcodeOccur[most[7]] << std::endl
//			<< "\t" << getOpcodeName(0, most[8]) << " - " << Stats::opcodeOccur[most[8]];
//		text.setString(ss.str());
//	} else if (infoDispMode == 1) {
//		ss << actualFPS << std::endl << "mode: " << static_cast<int>(_graphicMode) << std::endl << "CRAM at "
//			<< std::hex << std::setfill('0')
//			<< std::setw(4) << getColorTableAddress() << ": " << std::endl
//			<< std::endl;
//		//<< "VRAM pointer: " << std::setw(4) << _addressVRAM;
//		text.setString(ss.str());
//	}

//	sf::Text textReg;
//	textReg.setFont(font);
//	textReg.setCharacterSize(22);
//	textReg.setFillColor(sf::Color::Blue);
//	textReg.setPosition(_winInfo->getSize().x - 150, 0);
//	std::ostringstream ssReg;
//	ssReg << "Status: " << std::hex << uint16_t(_statusRegister) << std::endl << std::endl;
//	for (int i = 0; i < GRAPHIC_REGISTER_SIZE; i++)
//		ssReg << "#" << i << ": " << std::hex << uint16_t(getRegister(i)) << std::endl;
//	textReg.setString(ssReg.str());

//	sf::Text textFlag;
//	textFlag.setFont(font);
//	textFlag.setCharacterSize(22);
//	textFlag.setFillColor(sf::Color::Blue);
//	textFlag.setPosition(10, _winInfo->getSize().y - 30);
//	std::ostringstream ssFlag;
//	uint8_t r = CPU::Instance()->getRegisterFlag();
//	ssFlag << std::hex << "Flag: S=" << ((r >> 7) & 1) << " Z=" << ((r >> 6) & 1);
//	ssFlag << std::hex << " N=" << ((r >> 4) & 1) << " P/V=" << ((r >> 2) & 1);
//	ssFlag << std::hex << " N=" << ((r >> 1) & 1) << " C=" << ((r >> 0) & 1);
//	textFlag.setString(ssFlag.str());


//	sf::Text textDebugger;
//	textDebugger.setFont(font);
//	textDebugger.setCharacterSize(22);
//	textDebugger.setFillColor(sf::Color::Blue);
//	textDebugger.setPosition(10, _winInfo->getSize().y - 60);
//	if (Debugger::Instance()->getState() == Debugger::State::kRunning) {
//		if (_runningBarState == 0) {
//			textDebugger.setString(" |");
//			_runningBarState++;
//		} else if (_runningBarState == 1) {
//			textDebugger.setString(" /");
//			_runningBarState++;
//		} else if (_runningBarState == 2) {
//			textDebugger.setString("---");
//			_runningBarState++;
//		} else {
//			textDebugger.setString(" \\");
//			_runningBarState = 0;
//		}
//	} else {
//		textDebugger.setFillColor(sf::Color::Red);
//		textDebugger.setString("||");
//	}

//	_winInfo->clear(sf::Color::Black);
//	_winInfo->draw(text);
//	_winInfo->draw(textReg);
//	_winInfo->draw(textFlag);
//	_winInfo->draw(textDebugger);
//	_winInfo->display();
}

void GraphicsThread::drawGame()
{
#if GRAPHIC_PRECISE_TIMING
	long double decimalPixelNumber = _cpuStatesExecuted * (Graphics::PixelFrequency / CPU::Frequency);
	int nbPixel = static_cast<int>(decimalPixelNumber);
	_cpuStatesExecuted = (decimalPixelNumber - nbPixel) / (Graphics::PixelFrequency / CPU::Frequency);
# else
	std::chrono::duration<double, std::micro> intervalPixelTimer = std::chrono::steady_clock::now() - _pixelTimer;
	int nbPixel = intervalPixelTimer.count() / ((1.0 / 5.37624) / TIME_SCALE);
#endif

	for (; nbPixel > 0; --nbPixel) {
		_hCounter++;
		if (_hCounter == 1) {
			drawLine(_vCounter);
		}
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
				_tempLineRegister = getRegister(0xA);
				_lineInterruptCounter = _tempLineRegister;
			}
			if (_vCounter == 193) {
				setStatusRegisterBit(VDP::S_F, 1);
				drawFrame();
				_frameCounter++;
			}
			if (_vCounter >= 262) { // NTSC
			//if (_vCounter >= 313) { // PAL
				_vCounter = 0;
			}
		}
		_pixelTimer = std::chrono::steady_clock::now();
	}

	setIsSynchronized(true);
}

void GraphicsThread::resetLineInterrupt()
{
	std::scoped_lock lock{ _mutexData };
	_lineInterruptFlag = false;
}

uint16_t GraphicsThread::getCounterV()
{
	std::scoped_lock lock{ _mutexData };
	return _vCounter;
}

uint8_t GraphicsThread::getRegister(uint8_t iIndex)
{
	std::scoped_lock lock{ _mutexData };
	return _register[iIndex];
}

uint8_t GraphicsThread::getStatusRegister()
{
	std::scoped_lock lock{ _mutexData };
	return _statusRegister;
}

uint8_t GraphicsThread::getStatusRegisterBit(uint8_t position)
{
	std::scoped_lock lock{ _mutexData };
	return getBit8(_statusRegister, position);
}

uint8_t GraphicsThread::getVram(uint16_t iAddress)
{
	std::scoped_lock lock{ _mutexData };
	return _vram[iAddress];
}

void GraphicsThread::setCram(uint16_t iAddress, uint8_t iNewValue)
{
	std::scoped_lock lock{ _mutexData };
	*getColorTable(iAddress & 0b11111) = iNewValue;
}

void GraphicsThread::setGraphicMode(uint8_t iNewMode)
{
	std::scoped_lock lock{ _mutexData };
	_graphicMode = iNewMode;
}

void GraphicsThread::setRegister(uint8_t iIndex, uint8_t iNewValue)
{
	std::scoped_lock lock{ _mutexData };
	_register[iIndex] = iNewValue;
}

void GraphicsThread::setStatusRegister(uint8_t iNewValue)
{
	std::scoped_lock lock{ _mutexData };
	_statusRegister = iNewValue;
}

void GraphicsThread::setStatusRegisterBit(uint8_t position, bool newBit)
{
	std::scoped_lock lock{ _mutexData };
	setBit8(&_statusRegister, position, newBit);
}

void GraphicsThread::setVram(uint16_t iAddress, uint8_t iNewValue)
{
	std::scoped_lock lock{ _mutexData };
	_vram[iAddress] = iNewValue;
}

void GraphicsThread::setGameWindow(GameWindow *iWindow)
{
    _gameWindow = iWindow;
}

// ** PRIVATE **

uint8_t* const GraphicsThread::getNameTable(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getNameTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const GraphicsThread::getColorTable(uint8_t memoryOffset)
{
	return _cram + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const GraphicsThread::getColorTableMode2(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getColorTableAddressMode2()) + static_cast<uintptr_t>(memoryOffset);
}

// PATTERN GENERATOR TABLE
uint8_t* const GraphicsThread::getPatternGenerator(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getPatternGeneratorAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const GraphicsThread::getPatternGeneratorMode2(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getPatternGeneratorAddressMode2()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const GraphicsThread::getPatternGeneratorMode4(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getPatternGeneratorAddressMode4()) + static_cast<uintptr_t>(memoryOffset);
}

//SPRITE ATTRIBUTE TABLE
uint8_t* const GraphicsThread::getSpriteAttributeTable(uint8_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getSpriteAttributeTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}

uint8_t* const GraphicsThread::getSpritePatternTable(uint16_t memoryOffset)
{
	return _vram + static_cast<uintptr_t>(getSpritePatternTableAddress()) + static_cast<uintptr_t>(memoryOffset);
}


void GraphicsThread::drawFrame()
{
    uint8_t backdropColor = getColorTable(VDP::ColorBank::kFirst)[getBackdropColor()];

    if(isActiveDisplay()) {
        // TODO: draw palettes and patterns (if Inputs::getInfoDisplayMode())
        _frame.setBackdropColor(byteToPixelColor(backdropColor));
        _gameWindow->drawFrame(_frame);
    } else {
        _gameWindow->drawFrame(Frame{_frame.width(), _frame.height(), byteToPixelColor(backdropColor)});
    }
}

void GraphicsThread::drawLine(int line)
{
	int graphicMode = _graphicMode;
	if (int y = line; y < ImageHeight)
	{
		const uint8_t verticalScroll = getRegister(9);
		for (int x = 0; x < ImageWidth; x++)
		{
			const bool allowVScroll = !getBit8(getRegister(0), 7) || (x < 192);
			const uint8_t vTileScroll = allowVScroll ? (verticalScroll >> 3) : 0;
			const uint8_t vFineScroll = allowVScroll ? (verticalScroll & 0b0000'0111) : 0;

			const uint_fast8_t characterY = (vTileScroll + ((y + vFineScroll) / 8)) % 28; // TODO: 240-line
			const uint8_t rowPatternIndex = ((y+vFineScroll) % 8) * 4;
			if (graphicMode == 0)
			{
				const uint_fast8_t characterX = x / 8;
				const uint_fast16_t characterPosition = characterX + characterY * 32;
				uint8_t patternBaseAddress = getNameTable()[characterPosition];

				uint8_t bitPixel = getBit8(getPatternGenerator(8 * patternBaseAddress)[y - characterY], 7 - (x - characterX));
				uint_fast8_t colorByte = (getColorTable())[patternBaseAddress / 8];
//				_drawImage.setPixel(x, y, nibbleToSfmlColor((bitPixel ? colorByte >> 4 : colorByte & 0xF)));
                _frame.setPixel(x, y, nibbleToPixelColor(bitPixel ? colorByte >> 4 : colorByte & 0xF));
			} 
			else if (graphicMode == 2)
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
//				_drawImage.setPixel(x, y, nibbleToSfmlColor((bitPixel ? colorByte >> 4 : colorByte & 0xF)));
                _frame.setPixel(x, y, nibbleToPixelColor((bitPixel ? colorByte >> 4 : colorByte & 0xF)));
			} 
			else if (graphicMode == 4)
			{
				const uint8_t horizontalScroll = getRegister(8);
				const bool allowHScroll = !getBit8(getRegister(0), 6) || (x > 15);
				const uint8_t hTileScroll = allowHScroll ? (horizontalScroll >> 3) : 0;
				const uint8_t hFineScroll = allowHScroll ? (horizontalScroll & 0b0000'0111) : 0;

				if (allowHScroll && getBit8(getRegister(0), 5) && x < 8) {
//					_drawImage.setPixel(x, y, byteToSfmlColor(getColorTable(VDP::ColorBank::kFirst)[getBackdropColor()], true));
                    _frame.setPixel(x, y, byteToPixelColor(getColorTable(VDP::ColorBank::kFirst)[getBackdropColor()], true));
				} else {
					const uint_fast8_t characterX = ((32 - hTileScroll) + ((x - hFineScroll) / 8)) % 32;
					const uint_fast16_t characterPosition = characterX + characterY * 32;
					uint_fast16_t wordName = (getNameTable(characterPosition * 2)[1] << 8) | getNameTable(characterPosition * 2)[0];
					uint16_t patternIndex = (wordName & 0x1FF) * 32;

					wordName >>= 9; // Care about using it in the following lines
					uint_fast8_t flipH = wordName & 1;
					wordName >>= 1;
					uint_fast8_t flipV = wordName & 1;
					wordName >>= 1;
					uint_fast8_t paletteSelect = wordName & 1;
					wordName >>= 1;
					uint_fast8_t priority = wordName & 1;

					uint8_t const* const pattern = getPatternGeneratorMode4(patternIndex);
					const uint8_t columnPatternIndex = 7 - ((x - hFineScroll) % 8);
					uint8_t columnPatternIndexFlipped = columnPatternIndex;
					uint8_t rowPatternIndexFlipped = rowPatternIndex;

					if (flipH) {
						columnPatternIndexFlipped = 7 - columnPatternIndex;
					}
					if (flipV) {
						rowPatternIndexFlipped = ((1 * 8) - 1) * 4 - rowPatternIndex;
					}
					uint8_t selectedColor = getBit8(pattern[rowPatternIndexFlipped], columnPatternIndexFlipped);
					selectedColor |= getBit8(pattern[rowPatternIndexFlipped + 1], columnPatternIndexFlipped) << 1;
					selectedColor |= getBit8(pattern[rowPatternIndexFlipped + 2], columnPatternIndexFlipped) << 2;
					selectedColor |= getBit8(pattern[rowPatternIndexFlipped + 3], columnPatternIndexFlipped) << 3;

					uint_fast8_t colorByte = getColorTable(paletteSelect << 4)[selectedColor];
//					_drawImage.setPixel(x, y, byteToSfmlColor(colorByte, true));
                    _frame.setPixel(x, y, byteToPixelColor(colorByte, true));
				}
			} 
			else
			{				
				SLOG(ldebug << "Unknown VDP mode (" << std::dec << graphicMode << ")");
			}
		}

		if (graphicMode == 4 && Inputs::Instance()->getDrawSprite()) // Sprite subsystem
		{
			// MODE 4
			uint8_t const* const spriteAttributeTable = getSpriteAttributeTable();
			std::vector<std::vector<bool>> collisionMap(ImageWidth, std::vector<bool>(ImageWidth, false));
			bool collision = getBit8(_statusRegister, VDP::S_C);
			std::vector<uint8_t> lineCounter(ImageWidth, 0);
			bool lineOverflow = getBit8(_statusRegister, VDP::S_OVR);

			for (uint_fast8_t indexSprite = 0; indexSprite < 64; indexSprite++) {
				int y = static_cast<int>(spriteAttributeTable[indexSprite]) + 1;
				int x = static_cast<int>(spriteAttributeTable[128 + indexSprite * 2 + 0]);
				uint16_t patternIndex = static_cast<uint16_t>(spriteAttributeTable[128 + indexSprite * 2 + 1]);

				uint8_t spriteHeight = 8;

				// TODO: 224 & 240 lines mode
				if (y == 0xD0) {
					break;
				}

				if (getBit8(getRegister(0), 3)) {
					x -= 8;
				}

				setBit16(&patternIndex, 8, getBit8(getRegister(6), 2));

				if (getBit8(getRegister(1), 1)) {
					spriteHeight = 16;
				}

				if ((getRegister(1) & 1) == 1) {
					NOT_IMPLEMENTED("Scaled Sprite");
					continue; // TODO
				}

				for (int ySprite = 0; ySprite < spriteHeight; ++ySprite) {

					int yReal = y + ySprite;

					if (yReal != line) {
						continue;
					}

					if (yReal >= ImageHeight) {
						break;
					}

					lineCounter[yReal]++;
					if (lineCounter[yReal] > 8) {
						break;
					}

					if (getBit8(getRegister(1), 1) && ySprite >= (spriteHeight / 2)) {
						patternIndex += 1;
					}

					for (int xSprite = 0; xSprite < 8; ++xSprite) {
						int xReal = x + xSprite;
						if (xReal < 0 || xReal >= ImageWidth) {
							break;
						}

						uint8_t const* const pattern = getPatternGeneratorMode4(patternIndex * 32);
						const uint8_t rowPatternIndex = ((spriteHeight == 8) ? ySprite : ySprite % 8) * 4;
						const uint8_t columnPatternIndex = 7 - xSprite;
						uint8_t selectedColor = getBit8(pattern[rowPatternIndex + 0], columnPatternIndex);
						selectedColor |= getBit8(pattern[rowPatternIndex + 1], columnPatternIndex) << 1;
						selectedColor |= getBit8(pattern[rowPatternIndex + 2], columnPatternIndex) << 2;
						selectedColor |= getBit8(pattern[rowPatternIndex + 3], columnPatternIndex) << 3;

						
						if (selectedColor != 0) {
							uint_fast8_t colorByte = getColorTable(VDP::ColorBank::kSecond)[selectedColor];
//							sf::Color pixelColor = byteToSfmlColor(colorByte, false);
//							_drawImage.setPixel(xReal, yReal, pixelColor);
                            _frame.setPixel(xReal, yReal, byteToPixelColor(colorByte, false));
							if (!collision) {
								if (collisionMap[xReal][yReal]) {
									collision = true;
								} else {
									collisionMap[xReal][yReal] = true;
								}
							}
						}
						// Debug purpose:
						//_drawImage.setPixel(xReal, yReal, sf::Color::Yellow);
					}
				}
			}

			setBit8(&_statusRegister, VDP::S_C, collision);

			if (lineCounter[line] > 8) {
				lineOverflow = true;
			}
			setBit8(&_statusRegister, VDP::S_OVR, lineOverflow);
		}
	}
}

void GraphicsThread::drawPatterns()
{
//	int width = 256;
//	int height = 128;

//	sf::Image aImg;
//	aImg.create(width, height, sf::Color(0, 0, 0));
//	sf::Texture aTex;
//	aTex.create(width, height);

//	for (int y = 0; y < height; y++)
//	{
//		const uint8_t characterY = y / 8;
//		const uint8_t rowPatternIndex = (y - (static_cast<uint16_t>(characterY) * 8)) << 2; // "<<" for faster "*"
//		for (int x = 0; x < width; x++)
//		{
//			const uint8_t characterX = x / 8;
//			uint16_t patternIndex = characterX + characterY * 32;

//			patternIndex <<= 5; // = patternIndex * 32

//			uint8_t const* const pattern = &_vram[patternIndex];

//			const uint8_t columnPatternIndex = 7 - (x - (static_cast<uint16_t>(characterX) * 8));
//			uint8_t selectedColor = getBit8(pattern[rowPatternIndex], columnPatternIndex);
//			selectedColor |= getBit8(pattern[rowPatternIndex + 1], columnPatternIndex) << 1;
//			selectedColor |= getBit8(pattern[rowPatternIndex + 2], columnPatternIndex) << 2;
//			selectedColor |= getBit8(pattern[rowPatternIndex + 3], columnPatternIndex) << 3;

//			uint8_t colorByte = getColorTable(VDP::ColorBank::kFirst)[selectedColor];
//			aImg.setPixel(x, y, byteToSfmlColor(colorByte, true));
//		}
//	}
//	aTex.loadFromImage(aImg);
//	sf::Sprite aSprite(aTex);
//	aSprite.setPosition(0, 0);

//	_winGame->draw(aSprite);
}

void GraphicsThread::drawPalettes()
{
//	int width = 256;
//	int height = 192;

//	sf::Image aImg;
//	aImg.create(256, 32, sf::Color(255, 0, 0));
//	sf::Texture aTex;
//	aTex.create(256, 32);

//	for (uint_fast16_t y = 0; y < 32; y++)
//	{
//		for (uint_fast16_t x = 0; x < 256; x++)
//		{
//			uint_fast8_t patternPosX = x / 16;
//			uint_fast8_t patternPosY = y / 16;
//			uint_fast8_t colorByte = getColorTable(patternPosY * 16)[patternPosX];

//			if (x % 16 == 0 /*|| (x + 1) % 16 == 0*/ || x == 255 || y % 16 == 0 || y == 31/*|| (y + 1) % 16 == 0*/) {
//				aImg.setPixel(x, y, sf::Color::White);
//			} else {
//				aImg.setPixel(x, y, byteToSfmlColor(colorByte));
//			}
//		}
//	}
//	aTex.loadFromImage(aImg);
//	sf::Sprite aSprite(aTex);
//	aSprite.setPosition(0, 200);

//	_winGame->draw(aSprite);
}

void GraphicsThread::runThreadGame()
{
//    sf::RenderWindow windowGame(sf::VideoMode(GRAPHIC_WIDTH * Graphics::RatioSize * 2.0, GRAPHIC_HEIGHT * Graphics::RatioSize * 2.0), "Game - EmulIX MasterSystem");
//    windowGame.setPosition(sf::Vector2i(windowGame.getPosition().x + GRAPHIC_WIDTH * Graphics::RatioSize * 2.0 / 2, windowGame.getPosition().y));
//    _winGame = &windowGame;

//	if (_winGame) {
//		_winGame->clear(sf::Color::Black);
//		_winGame->display();
//		_winGame->requestFocus();
//	}

//	while (_isRunning) {
//		if (_requestWinGameActivation && _winGame && _winGame->isOpen()) {
//			_winGame->setActive(true);
//			_requestWinGameActivation = false;
//		}

//		if (_winGame && _winGame->isOpen()) {
//			Inputs::Instance()->captureEventsGame(_winGame);
//			if (_winGame->isOpen()) {
//				drawGame();
//			}
//		}

//		// Wait 1ms or new states
//		{
//			std::unique_lock<std::mutex> aLock{ this->mutexSync };
//			this->conditionSync.wait_for(aLock, 1ms, [this] { return !this->isSynchronized(); });
//		}
//	}

    while(_isRunning) {

        if(_gameWindow) {
            drawGame();
        }

        {
            std::unique_lock<std::mutex> aLock{ this->mutexSync };
            this->conditionSync.wait_for(aLock, 1ms, [this] { return !this->isSynchronized(); });
        }
    }
}

void GraphicsThread::runThreadInfo()
{
//	sf::RenderWindow windowInfo(sf::VideoMode(static_cast<int>(GRAPHIC_WIDTH * 2), static_cast<int>(GRAPHIC_HEIGHT * 2)), "Info - EmulIX MasterSystem");
//	windowInfo.setPosition(sf::Vector2i(windowInfo.getPosition().x - GRAPHIC_WIDTH * 2 / 2, windowInfo.getPosition().y));
//	_winInfo = &windowInfo;

//	while (_isRunning) {
//		if (_requestWinInfoActivation && _winInfo && _winInfo->isOpen()) {
//			_winInfo->setActive(true);
//			_requestWinInfoActivation = false;
//		}

//		if (_winInfo && _winInfo->isOpen()) {
//			Inputs::Instance()->captureEventsInfo(_winInfo);
//			if (_winInfo->isOpen()) {
//				drawInfo();
//			}
//		}

//		std::this_thread::sleep_for(4ms);
//	}
}
