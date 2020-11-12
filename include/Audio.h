#ifndef _H_EMULATOR_AUDIO
#define _H_EMULATOR_AUDIO

#include <bitset>

#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include "Log.h"

/* SN76489 chip emulation */
class Audio
{

public:
   static bool soundActive;

	Audio();

    u8 read(u8 port);
    u8 write(u8 port, u8 data);

	void run();

private:

    u8 _registerVol[4];  // Volume
    u16 _registerTone[4]; // Tone/noise
	sf::Int16 _output[4];
	float _counter[4];

    u8 _registerLatch;

	int _inputClock;

	// SFML
    sf::Clock _clockTime;
    sf::SoundBuffer _buffer[4];
    sf::Sound _sound[4];


	/// functions:
    void playSound(u8 indice);

};

#endif
