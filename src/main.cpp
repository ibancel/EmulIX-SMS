#include <iostream>
#include <sstream>

#ifdef __linux__
#include <pthread.h>
#endif

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "Inputs.h"
#include "Cartridge.h"
#include "Graphics.h"
#include "CPU.h"
#include "Log.h"


using namespace std;

/// TODO classe mère pour les périphériques

int main()
{
	Graphics::ratioSize = 2.0f;

	//Log::typeMin = Log::ALL /*& (~Log::DEBUG)*/;
	Log::typeMin = Log::ALL;
	Log::exitOnWarning = true;
	//Log::exitOnError = false;

	sf::RenderWindow window(sf::VideoMode(GRAPHIC_WIDTH*Graphics::ratioSize, GRAPHIC_WIDTH*Graphics::ratioSize), "Emul - MasterSystem");
	Inputs inputs;
	Cartridge rom;
	Memory mem;
	Graphics g(&mem, &window);
	CPU cpu(&mem, &g, &rom);
	//rom.readFromFile("ROMS/Sonic the Hedgehog.sms");
	//rom.readFromFile("ROMS/zexall.sms");

	/*for(int i = 0 ; i < 260 ; i++)
      cout << hex << i << " : " << getOpcodeName(i) << endl;*/


	//sf::sleep(sf::Time(20));

    while (window.isOpen())
    {
        inputs.captureEvents(&window);

        window.clear(sf::Color::Black);

        cpu.cycle();

        g.draw();

        window.display();
    }

    return 0;
}
