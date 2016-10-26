#ifndef _H_EMULATOR_INPUTS
#define _H_EMULATOR_INPUTS

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "definitions.h"
#include "Singleton.h"

class Inputs : public Singleton<Inputs>
{

public:
    Inputs();


    void captureEvents(sf::RenderWindow *app);

private:


};

#endif
