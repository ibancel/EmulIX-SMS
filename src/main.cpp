#include <iostream>
#include <sstream>
#include <thread>

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
#include "Debugger.h""
#include "Breakpoint.h"


using namespace std;

int main(int argc, char *argv[])
{
    sf::RenderWindow windowGame;

    Graphics::RatioSize = 1.0f;

    // Example: Log::typeMin = Log::ALL & (~Log::DEBUG);
    #if DEBUG_MODE
        Log::printConsole = true;
        Log::printFile = false;
        Log::typeMin = Log::ALL;
    #else
        Log::typeMin = Log::NONE;
    #endif

    Log::exitOnWarning = true;

    sf::RenderWindow windowInfo(sf::VideoMode(static_cast<int>(GRAPHIC_WIDTH*2), static_cast<int>(GRAPHIC_HEIGHT*2)), "Info - EmulIX MasterSystem");
    sf::Vector2i actualWinPos = windowInfo.getPosition();
    windowInfo.setPosition(sf::Vector2i(actualWinPos.x-GRAPHIC_WIDTH*2/2, actualWinPos.y));
    Inputs *inputs = Inputs::Instance();
    Cartridge *cartridge = Cartridge::Instance();
    Memory *mem = Memory::Instance();

    Graphics *g = Graphics::Instance();
    g->setWindowInfo(&windowInfo);

    CPU *cpu = CPU::Instance();

    if(argc > 1) {
        cartridge->insert(argv[1]);
        windowGame.create(sf::VideoMode(GRAPHIC_WIDTH*Graphics::RatioSize*2.0, GRAPHIC_HEIGHT*Graphics::RatioSize*2.0), "Game - EmulIX MasterSystem");
        actualWinPos = windowGame.getPosition();
        windowGame.setPosition(sf::Vector2i(actualWinPos.x+GRAPHIC_WIDTH*Graphics::RatioSize*2.0/2, actualWinPos.y));
        g->setWindowGame(&windowGame);
    }


    Debugger* debugger = Debugger::Instance();

    // Example of breakpoints:
    //debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kNumInstruction, 1234));
    //debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kAddress, 0x4321));

    cpu->init();

    constexpr double microsecondPerState = 1.0 / 3.58;

    std::chrono::time_point<std::chrono::steady_clock> chronoSync = chrono::steady_clock::now();
    chrono::duration<double, std::micro> intervalCycle;
    while (windowInfo.isOpen())
    {
        int nbTStates = 0;

        inputs->captureEventsInfo(&windowInfo);

        if (debugger->manage(cpu->getProgramCounter()) == Debugger::State::kRunning) {
            nbTStates = cpu->cycle();
        } else {
            Log::typeMin = Log::ALL;
            Log::printConsole = true;
            //Log::printFile = true;
            std::this_thread::sleep_for(1ms);
        }

        g->drawInfo();

        if(windowGame.isOpen()) {
            inputs->captureEventsGame(&windowGame);
            g->drawGame();
        }

        //if (nbTStates == 0) {
        //    nbTStates = 4;
        //}

        do {
            intervalCycle = chrono::steady_clock::now() - chronoSync;
        } while (intervalCycle.count() < nbTStates * microsecondPerState);
        chronoSync = chrono::steady_clock::now();
    }

    return 0;
}
