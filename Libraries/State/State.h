/* This library  works specifically with the Heatband. It stores all the data 
concerning the physical state that the heatband is in i.e. temperature,  
Battery Capacity Consumed, Cell voltages, Current flow through the Battery.
All measurements are updated in one funciton update().
capacpit consumed is calculated as an integral of the current flow

*/
#ifndef State_h
#define State_h

#include "OneWire.h"
#include "DallasTemperature.h"

class StateManager {
public:
	StateManager(byte voltPin1, 
		byte vPin2,
 		byte vPin3,
 		byte mampPin,
 		byte tBusPin);
	void update();													//updates temp, current, vCell, battery consumed 
	unsigned long get_zoneTemps(int index);
	float get_capacityConsumed();
	float get_current();
	float get_v1();
	float get_v2();
	float get_v3();
	float get_batteryCapacity();
	float get_batteryPercent();
	unsigned long get_TempAverage();
	void reset_capacityConsumed();
	void begin(unsigned long savedCapacity);

private:
	void initialiseArrays();
	void updateCapacityConsumed();
	void sampleSensors();
	void getTemps();

static const byte arraySize = 30;               //Size for Sensor arrays
const float cellDangerThreshold = 4.2;
const byte voltPin1;
const byte voltPin2;
const byte voltPin3;
const byte milliampPin;
const byte tempBusPin;


//variables for measuring current
unsigned int currentArray[arraySize];
float current = 0;

//VARIABLES for measuring voltage
unsigned int vCell1Array[arraySize];
unsigned int vCell2Array[arraySize];
unsigned int vCell3Array[arraySize];
float v1 = 0;
float v2 = 0;
float v3 = 0;

//VARIABLES for temp control
OneWire oneWire;
DallasTemperature sensors;
DeviceAddress zone1, zone2, zone3;
byte tempRes = 9;
unsigned long zoneTemps[3];
long lzoneTemps[3];
unsigned long TempAverage;

//VARIABLES for battery
const float batteryCapacity = 2.6*2*3600;         //battery capacity in As
float capacityConsumed = 0;                   



};


#endif