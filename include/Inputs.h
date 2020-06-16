#ifndef _H_EMULATOR_INPUTS
#define _H_EMULATOR_INPUTS

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "definitions.h"
#include "Singleton.h"
#include "Debugger.h"

class Graphics;

class Inputs : public Singleton<Inputs>
{
public:
    
    enum JoypadId : size_t { kJoypad1 = 0, kJoypad2 = 1 };
    enum ControllerKey : size_t { CK_UP = 0, CK_DOWN = 1, CK_RIGHT = 2, CK_LEFT = 3, CK_FIREA = 4, CK_FIREB = 5 };

    Inputs();


    void captureEventsInfo(sf::RenderWindow *app);
    void captureEventsGame(sf::RenderWindow *app);

    // idController is 0 for Joypad 1 and 1 for Joypad 2
    bool controllerKeyPressed(JoypadId idController, ControllerKey key);

    void switchDrawSprite();
    void switchInfoDisplayMode();

    bool getDrawSprite();
    int getInfoDisplayMode();

    bool isStopRequested();

private:
    sf::Keyboard::Key _userKeys[6];
    Debugger* _debugger;
    Graphics* _graphics;

    bool _controller[2][6];
    bool _drawSprite;
    bool _isStopRequested;

    int _infoDisplayMode;
};

#endif
