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
#include "Debugger.h"
#include "Breakpoint.h"


using namespace std;

int main(int argc, char *argv[])
{
    sf::RenderWindow windowGame;

    Graphics::RatioSize = 1.0f;

    // Example: Log::typeMin = Log::ALL & (~Log::DEBUG);
#if DEBUG_MODE
    Log::printConsole = false;
    Log::printFile = true;
    Log::typeMin = Log::ALL;
#else
    Log::typeMin = Log::NONE;
#endif

    Log::exitOnWarning = true;

    Inputs *inputs = Inputs::Instance();
    Cartridge *cartridge = Cartridge::Instance();
    Memory *mem = Memory::Instance();

    Graphics *g = Graphics::Instance();
#if !GRAPHIC_THREADING
    sf::RenderWindow windowInfo(sf::VideoMode(static_cast<int>(GRAPHIC_WIDTH*2), static_cast<int>(GRAPHIC_HEIGHT*2)), "Info - EmulIX MasterSystem");
    sf::Vector2i actualWinPos = windowInfo.getPosition();
    windowInfo.setPosition(sf::Vector2i(actualWinPos.x-GRAPHIC_WIDTH*2/2, actualWinPos.y));
    g->setWindowInfo(&windowInfo);
#endif



    CPU *cpu = CPU::Instance();

    if(argc > 1) {
        cartridge->insert(argv[1]);

#if !GRAPHIC_THREADING
        windowGame.create(sf::VideoMode(GRAPHIC_WIDTH*Graphics::RatioSize*2.0, GRAPHIC_HEIGHT*Graphics::RatioSize*2.0), "Game - EmulIX MasterSystem");
        actualWinPos = windowGame.getPosition();
        windowGame.setPosition(sf::Vector2i(actualWinPos.x+GRAPHIC_WIDTH*Graphics::RatioSize*2.0/2, actualWinPos.y));
        g->setWindowGame(&windowGame);
 #endif
    }

    Debugger* debugger = Debugger::Instance();
    // Example of breakpoints:
    //debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kNumInstruction, 1234));
    //debugger->addBreakpoint(std::make_unique<Breakpoint>(Breakpoint::Type::kAddress, 0x4321));

    cpu->init();

    double offsetTiming = 0.0;

    std::chrono::time_point<std::chrono::steady_clock> chronoSync = chrono::steady_clock::now();
    chrono::duration<long double, std::micro> intervalCycle;
    while (!inputs->isStopRequested())
    {
        int nbTStates = 0;


        uint16_t pc = cpu->getProgramCounter();
        if (debugger->manage(cpu->getProgramCounter()) == Debugger::State::kRunning) {
            nbTStates = cpu->cycle();
            debugger->addNumberCycle(nbTStates);
        } else {
            Log::typeMin = Log::ALL;
            Log::printConsole = true;
            //Log::printFile = true;
            std::this_thread::sleep_for(1ms);
        }

#if !GRAPHIC_THREADING
        if (windowInfo.isOpen()) {
            inputs->captureEventsInfo(&windowInfo);
            g->drawInfo();
        }

        if(windowGame.isOpen()) {
            inputs->captureEventsGame(&windowGame);
            g->drawGame();
        }
#endif

        const long double expectedExecTime = nbTStates * CPU::MicrosecondPerState;

        intervalCycle = chrono::steady_clock::now() - chronoSync;
        Stats::addExecutionStat(nbTStates, intervalCycle.count());

        while (intervalCycle.count() < expectedExecTime) {
            intervalCycle = chrono::steady_clock::now() - chronoSync;
        }

        chronoSync = chrono::steady_clock::now();
    }

    g->stopRunning();

    return 0;
}
