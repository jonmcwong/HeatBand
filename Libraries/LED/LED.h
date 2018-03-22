//for control colur and blinking of the LED

#ifndef LED_h
#define LED_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class LED {
public:
	LED(unsigned long primary, 
			unsigned long secondary,
			byte rLedPin,
			byte gLedPin);
	void blink(unsigned long mainTimeKeeper,int mode,int averagedSensorTemperature,int maxTemperatureChange);

private:
	void lightUp(int averagedSensorTemperature,int maxTemperatureChange);

	const unsigned long primaryBlinkRate;
	const unsigned long secondaryBlinkRate;
	const byte redLedPin;
	const byte greenLedPin;
	int numberOfBlinks = 0;
	unsigned long primaryLocalTimeKeeper = 0;
	unsigned long secondaryLocalTimeKeeper = 0;
	bool toggle1 = false;
	bool toggle2 = false;

};

#endif