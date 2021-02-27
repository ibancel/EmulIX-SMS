#pragma once

#include <bitset>
#include <chrono>

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
    s16 _output[4];
	float _counter[4];

    u8 _registerLatch;

	int _inputClock;

    std::chrono::time_point<std::chrono::steady_clock> _clockTime;
//    sf::SoundBuffer _buffer[4];
//    sf::Sound _sound[4];


	/// functions:
    void playSound(u8 indice);

};
