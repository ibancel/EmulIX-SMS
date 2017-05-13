#ifndef _H_EMULATOR_INPUTS
#define _H_EMULATOR_INPUTS

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "definitions.h"
#include "Singleton.h"


enum CONTROLLER_KEYS { CK_UP = 0, CK_DOWN = 1, CK_RIGHT = 2, CK_LEFT = 3, CK_FIREA = 4, CK_FIREB = 5 };

class Inputs : public Singleton<Inputs>
{

public:
    Inputs();


    void captureEvents(sf::RenderWindow *app);

    // idController is 0 for Joypad 1 and 1 for Joypad 2
    bool controllerKeyPressed(uint8_t idController, CONTROLLER_KEYS key);

private:
    bool _controller[6];
    sf::Keyboard::Key _userKeys[6];

};

#endif
