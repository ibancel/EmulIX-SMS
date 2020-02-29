#include "Inputs.h"

#include "Graphics.h"



Inputs::Inputs() : _debugger{ Debugger::Instance() }, _graphics{ Graphics::Instance() }, _controller{ false }
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
		if (event.type == sf::Event::Closed)
			app->close();
		if(event.type == sf::Event::KeyReleased)
		{
			if(event.key.code == sf::Keyboard::Escape) {
				app->close();
			}
			if (event.key.code == sf::Keyboard::Numpad0 || event.key.code == sf::Keyboard::Num0) {
				_graphics->switchInfoPrintMode();
			}
			if (event.key.code == sf::Keyboard::V) {
				_graphics->dumpVram();
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
		if (event.type == sf::Event::Closed)
			app->close();
        if(event.type == sf::Event::KeyPressed)
        {
			for (int i = 0; i < 6; i++) {
				if (event.key.code == _userKeys[i]) {
					_controller[JoypadId::kJoypad1][i] = true;
				}
			}
        }
		if(event.type == sf::Event::KeyReleased)
		{
			for (int i = 0; i < 6; i++) {
				if (event.key.code == _userKeys[i]) {
					_controller[JoypadId::kJoypad1][i] = false;
				}
			}

			if (event.key.code == sf::Keyboard::Escape) {
				app->close();
			}
			if (event.key.code == sf::Keyboard::Numpad0 || event.key.code == sf::Keyboard::Num0) {
				_graphics->switchInfoPrintMode();
			}
			_debugger->captureEvents(event);
		}

	}
}

bool Inputs::controllerKeyPressed(JoypadId idController, ControllerKey cKey)
{
    return _controller[idController][cKey];
}
