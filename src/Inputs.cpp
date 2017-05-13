#include "Inputs.h"



Inputs::Inputs()
{
    for(int i = 0 ; i < 6 ; i++)
        _controller[i] = false;

    _userKeys[CK_UP]    = sf::Keyboard::Up;
    _userKeys[CK_DOWN]  = sf::Keyboard::Down;
    _userKeys[CK_RIGHT] = sf::Keyboard::Right;
    _userKeys[CK_LEFT]  = sf::Keyboard::Left;
    _userKeys[CK_FIREA] = sf::Keyboard::D;
    _userKeys[CK_FIREB] = sf::Keyboard::F;
}


void Inputs::captureEvents(sf::RenderWindow *app)
{
	sf::Event event;
	while (app->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			app->close();
        if(event.type == sf::Event::KeyPressed)
        {
            for(int i = 0 ; i < 6 ; i++)
                if(event.key.code == _userKeys[i])
                    _controller[i] = true;
        }
		if(event.type == sf::Event::KeyReleased)
		{
		    for(int i = 0 ; i < 6 ; i++)
                if(event.key.code == _userKeys[i])
                    _controller[i] = false;

			if(event.key.code == sf::Keyboard::Escape) {
				app->close();
			}
			else if(event.key.code == sf::Keyboard::Space) {
				if(!STEP_BY_STEP)
					systemPaused = !systemPaused;
				else
					systemStepCalled = true;
			}
		}
	}
}

bool Inputs::controllerKeyPressed(uint8_t idController, CONTROLLER_KEYS cKey)
{
    return _controller[cKey];
}
