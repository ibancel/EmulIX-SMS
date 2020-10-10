#include <cmath>

#include "Audio.h"

using namespace std;

bool Audio::soundActive = false;

Audio::Audio()
{
	// initialisation
	for(int i = 0 ; i < 4 ; i++)
	{
		_registerVol[i] = 0xF;
		_registerTone[i] = 0;
		_output[i] = 1;
		_counter[i] = 0;
        _sound[i].setLoop(true);
	}

	_registerLatch = 0;
	_inputClock = 3579545;
}

/*
	follows the documentation of:
	http://www.smspower.org/uploads/Development/SN76489-20030421.txt?sid=abc441e2c56383ac7ece36d87afe0719
*/
uint8_t Audio::write(uint8_t port, uint8_t data)
{
	//slog << lerror << "audio: " << hex << (uint16_t)port << " | " << (uint16_t)data << endl;

	if(port == 0x7E || port == 0x7F)
	{
		if((data >> 7) == 1) // LATCH/DATA byte
		{
			_registerLatch = ((data >> 4) & 0b111); // it's save as: channel|channel|type

			// 1 for volume ; 0 for tone/noise
			if((_registerLatch & 1) == 1) {
				_registerVol[(_registerLatch & 0b110)>>1] = data & 0xF;
			}
			else
				_registerTone[(_registerLatch & 0b110)>>1] = data & 0xF;
		}
		else // DATA byte
		{
			if((_registerLatch & 1) == 1)
				_registerVol[(_registerLatch & 0b110)>>1] = data & 0xF;
			else
				_registerTone[(_registerLatch & 0b110)>>1] += ((data & 0b111111) << 4);
		}

		playSound((_registerLatch & 0b0110) >> 1);
	}

	return 0; // TODO
}

void Audio::run()
{
    sf::Time elapsedTime = _clockTime.restart();

	for(int i = 0 ; i < 1 ; i++)
	{
		continue;
        if(_counter[i] > 0)
            _counter[i] -= elapsedTime.asSeconds() * _inputClock;

		// no else for manage it directly at 0
		if(_counter[i] <= 0)
		{
			if(i < 3) // tone
			{
				_counter[i] = (_registerTone[i]!=0?_registerTone[i]:1);
				_output[i] = -_output[i];

			}
			else // noise
			{
				switch(_registerTone[i] & 0b11)
				{
					case 0:
						_counter[i] = 0x10;
						break;
					case 1:
						_counter[i] = 0x20;
						break;
					case 2:
						_counter[i] = 0x40;
						break;
					case 3:
						_counter[i] = _registerTone[2];
						break;
				}
			}
		}
	}
}

void Audio::playSound(uint8_t indice)
{
   if(!soundActive) return;

	sf::Int16 raw[44100];
	const double TWO_PI = 6.28318;
	double x = 0;
	double note = ((double)_inputClock/(32*_registerTone[0]));
	if(_registerTone[0] == 0)
		note = 1;
	note = 440;
	const double increment = note/44100;
	for(unsigned int i = 0 ; i < 44100 ; i++)
	{
		raw[i] = 3000/16 * _registerVol[indice] * sin(x*TWO_PI);
		int sign = 0;
		if(sin(x*TWO_PI) > 0)
			sign = 1;
		else if(sin(x*TWO_PI) < 0)
			sign = -1;
		//raw[i] = 3000/16 * _registerVol[indice] * sign;
		x += increment;
	}

	//cout << hex << (uint16_t)indice << " - " << (uint16_t)_registerVol[indice] << " - " << (uint16_t)_registerTone[indice] << endl;
    if (!_buffer[indice].loadFromSamples(raw, 44100, 1, 44100)) {
        std::cerr << "Loading failed!" << std::endl;
    }

    _sound[indice].stop();
    _sound[indice].setBuffer(_buffer[indice]);
    _sound[indice].play();
}
