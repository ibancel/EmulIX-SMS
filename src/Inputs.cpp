#include "Inputs.h"


Inputs::Inputs()
{

}


void Inputs::captureEvents(sf::RenderWindow *app)
{
	sf::Event event;
	while (app->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			app->close();
		if(event.type == sf::Event::KeyReleased)
		{
			if(event.key.code == sf::Keyboard::Escape)
				app->close();
         else if(event.key.code == sf::Keyboard::Space)
            systemPaused = !systemPaused;
		}
	}
}
