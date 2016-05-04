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

	uint8_t read(uint8_t port);
	uint8_t write(uint8_t port, uint8_t data);

	void run();

private:

	uint8_t _registerVol[4];  // Volume
	uint16_t _registerTone[4]; // Tone/noise
	sf::Int16 _output[4];
	float _counter[4];

	uint8_t _registerLatch;

	int _inputClock;

	// SFML
	sf::Clock _clockTime;
	sf::SoundBuffer _buffer[4];
	sf::Sound _sound[4];


	/// functions:
	void playSound(uint8_t indice);

};

#endif
