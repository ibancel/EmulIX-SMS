#ifndef _H_EMULATOR_INPUTS
#define _H_EMULATOR_INPUTS

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "definitions.h"

class Inputs
{

public:
    Inputs();


    void captureEvents(sf::RenderWindow *app);

private:


};

#endif
