#include "LED.h"

LED::LED(unsigned long primary, 
					unsigned long secondary,
					byte rLedPin,
					byte gLedPin)
: primaryBlinkRate(primary),
secondaryBlinkRate(secondary),
redLedPin(rLedPin),
greenLedPin(gLedPin)
 {

}
void LED::blink(unsigned long mainTimeKeeper,int mode,int averagedSensorTemperature,int maxTemperatureChange) {
	//blinks leds 1 time for mode 1, 2 times for mode 2 and 3 times for mode 3, 
   
  if ( (mainTimeKeeper - primaryLocalTimeKeeper) >= primaryBlinkRate)  //toggle for main blinking rate
  {
    primaryLocalTimeKeeper = millis();
    toggle1 = !toggle1;
    numberOfBlinks = mode * 2;    //heatBandMode
    toggle2 = false;
  }
  
    if (toggle1 && ((mainTimeKeeper - secondaryLocalTimeKeeper) >= secondaryBlinkRate)) //toggle for secondary blink rate
    {
      toggle2 = !toggle2;
      secondaryLocalTimeKeeper = millis();
      numberOfBlinks--;
    }

  if (toggle2 && numberOfBlinks > 0)   //turns on or off leds and keeps track of how many times the leds should turn off
  {
    lightUp(averagedSensorTemperature,maxTemperatureChange);
  }
  else
  { 
    digitalWrite(redLedPin,LOW);
    digitalWrite(greenLedPin,LOW);
  }
}

void LED::lightUp(int averagedSensorTemperature,int maxTemperatureChange) //TESTED AND WORKING 
//lights appropriate led colour by reading capacity consumed
{
  if (averagedSensorTemperature <= maxTemperatureChange/3)
  {
    digitalWrite(redLedPin,HIGH);
    digitalWrite(greenLedPin,LOW);
  }
  else if ((averagedSensorTemperature > (maxTemperatureChange/3)) && (averagedSensorTemperature <= 0.6*maxTemperatureChange))
  {
    digitalWrite(redLedPin,LOW);
    digitalWrite(greenLedPin,HIGH);
  }
  else  
  {
    digitalWrite(redLedPin,HIGH);
    digitalWrite(greenLedPin,HIGH);
  } 
} //END of light led