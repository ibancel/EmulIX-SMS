#include "Graphics.h"

#include <iomanip>

#include "GraphicsThread.h"
#include "System.h"

using namespace std;

float Graphics::RatioSize = 2.0f;

Graphics::Graphics(System& parent) : _memory { parent.getMemory() }
{
	_addressVRAM = 0;
	_codeRegister = 0;
	_actualMode = 0;
	_controlByte = 0;
	_controlCmd = 0;
	_readAheadBuffer = 0;

	_debugDrawSprite = true;
}

void Graphics::reset()
{
	_graphicsThread.close(true);
	_graphicsThread.reset();
	startRunning();

	// should verify if the following instructions works well:
	_count = 0;
	_readAheadBuffer = 0;
}

void Graphics::startRunning() { _graphicsThread.start(); }

void Graphics::stopRunning() { _graphicsThread.close(true); }

void Graphics::syncThread()
{
#if GRAPHIC_THREADING
	// Sync with graphics thread
	{
		std::unique_lock<std::mutex> aLock { _graphicsThread.mutexSync };
		_graphicsThread.conditionSync.wait(aLock, [&] { return _graphicsThread.isSynchronized(); });
	}
#endif
}

u8 Graphics::read(const u8 port)
{
	syncThread();

	switch(port) {
		case 0x7E: { // V counter
			u16 vCounter = _graphicsThread.getCounterV();
			return (vCounter > 0xDA ? vCounter - 6 : vCounter);
			// TODO other values NTSC/PAL
			return 0;
		} break;
		case 0x7F: { // H counter
			NOT_IMPLEMENTED("READ H COUNTER");
			return 0;
		} break;
		case 0xBE: { // data port
			_controlByte = 0;
			// TODO verify
			const u8 result = _readAheadBuffer;
			_readAheadBuffer = _graphicsThread.getVram(_addressVRAM);
			incrementVramAddress();
			return result;
		} break;
		case 0xBF: { // control port
			_controlByte = 0;
			const u8 retStatusRegister = _graphicsThread.getStatusRegister();
			_graphicsThread.setStatusRegisterBit(VDP::S_F, 0);
			_graphicsThread.setStatusRegisterBit(VDP::S_C, 0);
			if(getGraphicMode() != 2) {
				_graphicsThread.setStatusRegisterBit(VDP::S_OVR, 0);
			}
			_graphicsThread.resetLineInterrupt();
			return retStatusRegister;
		} break;
		default:
			SLOG_THROW(lwarning << hex << "Undefined Graphic port (" << static_cast<u16>(port) << ") ");
			break;
	}

	return 0;
}

u8 Graphics::write(const u8 port, const u8 data)
{
	if(port == 0xBE) // data port
	{
		// std::cout << hex << "[VDP] pc=" << CPU::instance()->getProgramCounter() << " | write data=" << (u16)data <<
		// " at " << _addressVRAM << " & coderegister=" << (u16)_codeRegister << endl;
		SLOG(ldebug << hex << "[VDP] write data=" << (u16)data << " at " << _addressVRAM
					<< " & coderegister=" << (u16)_codeRegister);
		if(_codeRegister == 3) {
			_graphicsThread.setCram(_addressVRAM & 0b11111, data);
		} else { // code == 0,1,2
			_graphicsThread.setVram(_addressVRAM, data);
		}
		incrementVramAddress();
		_readAheadBuffer = data;
		_controlByte = 0;
	} else if(port == 0xBF) // control port
	{
		if(_controlByte == 0) {
			_controlCmd = data;
			_controlByte++;
		} else {
			_controlCmd += (data << 8);
			_controlByte = 0;

			controlAction();
		}
	} else {
		slog << lwarning << hex << "Undefined Graphic port (" << static_cast<u16>(port) << ") " << endl;
	}

	return 0; // TODO
}

u8 Graphics::controlAction()
{
	_codeRegister = (_controlCmd >> 14);

	SLOG(ldebug << hex << "[VPD] control code:" << (u16)_codeRegister);

	if(_codeRegister != 2) {
		_addressVRAM = _controlCmd & 0b00111111'11111111;
		SLOG(ldebug << "[VDP] control new address: " << hex << _addressVRAM);
	}

	// std::cout << hex << "[VDP] pc=" << CPU::instance()->getProgramCounter() << " | control code=" <<
	// (u16)_codeRegister << " new address=" << _addressVRAM << endl;

	if(_codeRegister == 0) {
		// Read VRAM
		_readAheadBuffer = _graphicsThread.getVram(_addressVRAM);
		incrementVramAddress();
	} else if(_codeRegister == 1) {
		// Write VRAM, address is set: nothing more
	} else if(_codeRegister == 2) {
		u8 num = ((_controlCmd & 0xF00) >> 8);
		if(num < GRAPHIC_REGISTER_SIZE) {
			_graphicsThread.setRegister(num, _controlCmd & 0xFF);
		}

		SLOG(ldebug << hex << "[VDP] set r[" << (u16)num << "] = " << (u16)(_controlCmd & 0xFF));
		SLOG(ldebug << hex << "[VDP] mode:" << (u16)getBit8(_graphicsThread.getRegister(0), 2)
					<< (u16)getBit8(_graphicsThread.getRegister(1), 3)
					<< (u16)getBit8(_graphicsThread.getRegister(0), 1)
					<< (u16)getBit8(_graphicsThread.getRegister(1), 4));
	} else if(_codeRegister == 3) {
		// Read CRAM, address is set: nothing more
	} else {
		NOT_IMPLEMENTED("UNDEFINED CODE REGISTER");
	}

	updateGraphicMode();

	return 0; // TODO
}

// void Graphics::setWindowInfo(sf::RenderWindow *win)
//{
//	_graphicsThread.setWindowInfo(win);
//}

// void Graphics::setWindowGame(sf::RenderWindow *win)
//{
//	_graphicsThread.setWindowGame(win);
//}

void Graphics::setGameWindow(GameWindow* iWindow) { _graphicsThread.setGameWindow(iWindow); }

void Graphics::dumpVram()
{
	// TODO in GraphicsThread? => probably YES
	if(DumpMode == 0) {
		std::ofstream file("vdp_dump.txt", std::ios_base::out | std::ios_base::binary);
		for(int i = 0; i < GRAPHIC_VRAM_SIZE; i++) {
			file << _graphicsThread.getVram(i);
		}
		file.close();
	} else if(DumpMode == 1) {
		std::ofstream file("vdp_dump.txt", std::ios_base::out);
		for(int i = 0; i < GRAPHIC_VRAM_SIZE; i++) {
			file << std::hex << std::setfill('0') << std::uppercase << std::setw(2) << (u16)(_graphicsThread.getVram(i))
				 << " ";
			if(i != 0 && (i % 16) == 15) {
				file << std::endl;
			}
		}
	}
}

u8 Graphics::getGraphicMode() { return _actualMode; }

#if !GRAPHIC_THREADING
void Graphics::drawGame() { _graphicsThread.drawGame(); }

void Graphics::drawInfo() { _graphicsThread.drawInfo(); }
#endif

// ** PROTECTED **

void Graphics::updateGraphicMode()
{
#if MACHINE_VERSION == 1
	const bool mode1 = getBit8(_graphicsThread.getRegister(1), 4);
	const bool mode2 = getBit8(_graphicsThread.getRegister(0), 1);
	const bool mode3 = getBit8(_graphicsThread.getRegister(1), 3);
	const bool mode4 = getBit8(_graphicsThread.getRegister(0), 2);

	u8 aNewMode = _actualMode;

	if(mode4) {
		if(mode1 && mode2 && !mode3) {
			NOT_IMPLEMENTED("224-line display");
		} else if(!mode1 && mode2 && mode3) {
			NOT_IMPLEMENTED("240-line display");
		} else if(!mode1 || mode2) {
			aNewMode = 4;
		} else {
			aNewMode = 1;
		}
	} else {
		if(!mode3 && !mode2 && !mode1) {
			aNewMode = 0;
		} else if(!mode3 && !mode2 && mode1) {
			aNewMode = 1;
		} else if(!mode3 && mode2 && !mode1) {
			aNewMode = 2;
		} else if(mode3 && !mode2 && !mode1) {
			aNewMode = 3;
		} else {
			NOT_IMPLEMENTED("DOUBLE VIDEO MODE");
		}
	}

	if(aNewMode != _actualMode) {
		_actualMode = aNewMode;
		_graphicsThread.setGraphicMode(_actualMode);
	}
#else
	NOT_IMPLEMENTED("Machine version");
#endif
}
