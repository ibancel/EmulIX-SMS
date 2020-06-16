#include "Inputs.h"

#include "Graphics.h"



Inputs::Inputs() : 
	_debugger{ Debugger::Instance() }, 
	_graphics{ Graphics::Instance() }, 
	_controller{ false },
	_infoDisplayMode{ 0 },
	_drawSprite{ true },
	_isStopRequested{ false }
{
    _userKeys[ControllerKey::CK_UP]    = sf::Keyboard::Up;
    _userKeys[ControllerKey::CK_DOWN]  = sf::Keyboard::Down;
    _userKeys[ControllerKey::CK_RIGHT] = sf::Keyboard::Right;
    _userKeys[ControllerKey::CK_LEFT]  = sf::Keyboard::Left;
    _userKeys[ControllerKey::CK_FIREA] = sf::Keyboard::D;
    _userKeys[ControllerKey::CK_FIREB] = sf::Keyboard::F;
}


void Inputs::captureEventsInfo(sf::RenderWindow *app)
{
	sf::Event event;
	while (app->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			_isStopRequested = true;
			app->close();
		} else if(event.type == sf::Event::KeyReleased) {
			if(event.key.code == sf::Keyboard::Escape) {
				_isStopRequested = true;
				app->close();
			}
			if (event.key.code == sf::Keyboard::Numpad0 || event.key.code == sf::Keyboard::Num0) {
				switchInfoDisplayMode();
			}
			if (event.key.code == sf::Keyboard::V) {
				_graphics->dumpVram();
			}
			if (event.key.code == sf::Keyboard::R) {
				Memory::Instance()->dumpRam();
			}
			_debugger->captureEvents(event);
		}

	}
}

void Inputs::captureEventsGame(sf::RenderWindow *app)
{
	sf::Event event;
	while (app->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			app->close();
		} else if (event.type == sf::Event::KeyPressed) {
			for (int i = 0; i < 6; i++) {
				if (event.key.code == _userKeys[i]) {
					_controller[JoypadId::kJoypad1][i] = true;
				}
			}
        } else if(event.type == sf::Event::KeyReleased) {
			for (int i = 0; i < 6; i++) {
				if (event.key.code == _userKeys[i]) {
					_controller[JoypadId::kJoypad1][i] = false;
				}
			}

			if (event.key.code == sf::Keyboard::Escape) {
				app->close();
			}
			if (event.key.code == sf::Keyboard::Numpad0 || event.key.code == sf::Keyboard::Num0) {
				switchInfoDisplayMode();
			}
			if (event.key.code == sf::Keyboard::Numpad5 || event.key.code == sf::Keyboard::Num5) {
				switchDrawSprite();
			}
			_debugger->captureEvents(event);
		}

	}
}

bool Inputs::controllerKeyPressed(JoypadId idController, ControllerKey cKey)
{
    return _controller[idController][cKey];
}

void Inputs::switchDrawSprite()
{
	_drawSprite = !_drawSprite;
}

void Inputs::switchInfoDisplayMode()
{
	_infoDisplayMode++;
	if (_infoDisplayMode > 1) {
		_infoDisplayMode = 0;
	}
}

bool Inputs::getDrawSprite()
{
	return _drawSprite;
}

int Inputs::getInfoDisplayMode()
{
	return _infoDisplayMode;
}

bool Inputs::isStopRequested()
{
	return _isStopRequested;
}