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

int main(int argc, char *argv[])
{
    Graphics::ratioSize = 2.0f;

    //Log::typeMin = Log::ALL & (~Log::DEBUG);
    Log::typeMin = Log::ALL;
    Log::exitOnWarning = true;
    //Log::exitOnError = false;

    sf::RenderWindow window(sf::VideoMode(GRAPHIC_WIDTH*Graphics::ratioSize, GRAPHIC_WIDTH*Graphics::ratioSize), "Emul - MasterSystem");
    Inputs *inputs = Inputs::instance();
    Cartridge *cartridge = Cartridge::instance();
    Memory *mem = Memory::instance();

    Graphics *g = Graphics::instance();
    g->setWindow(&window);

    CPU *cpu = CPU::instance();

    if(argc > 1) {
		cartridge->readFromFile(argv[1]);
    }

	// Quick implementation of breakpoints
    vector<int16_t> breakpoints;


	bool toPause;
    while (window.isOpen())
    {
        inputs->captureEvents(&window);

        window.clear(sf::Color::Black);

		toPause = false;
        for(int i = 0 ; i < breakpoints.size() && !systemPaused ; i++) {
			if(cpu->getProgramCounter() == breakpoints[i]) {
				#if BREAKPOINT_STYLE == 0
				systemPaused = true;
				#elif BREAKPOINT_STYLE == 1
				toPause = true;
				#endif // BREAKPOINT_STYLE
				breakpoints.erase(breakpoints.begin()+i);
			}
        }

        /*if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			systemStepCalled = true;*/

        if(!systemPaused)
            cpu->cycle();

        g->draw();

		if(toPause)
			systemPaused = true;

        window.display();
    }

    return 0;
}
