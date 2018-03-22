#include "State.h"

StateManager::StateManager(byte vPin1, 
		byte vPin2,
 		byte vPin3,
 		byte mampPin,
 		byte tBusPin) 
: voltPin1(vPin1),
voltPin2(vPin2),
voltPin3(vPin3),
milliampPin(mampPin),
tempBusPin(tBusPin),
oneWire(tBusPin),
sensors(&oneWire)
{

};

void StateManager::begin(unsigned long savedCapacityConsumed) {
	initialiseArrays();
	sensors.begin();
	sensors.getAddress(zone1, 0);
	sensors.getAddress(zone2, 1);
	sensors.getAddress(zone3, 2);
	sensors.setResolution(zone1, tempRes);
  sensors.setResolution(zone2, tempRes);
  sensors.setResolution(zone3, tempRes);
  capacityConsumed = (float)savedCapacityConsumed/3;			//saved as amp seconds * 3 for resolution
}

	//***SMOOTHING ARRAY INITIALISATION****
void StateManager::initialiseArrays() {
  for (int i = 0; i < arraySize; i++) {
    currentArray[i] = analogRead (milliampPin); //analogRead(milliampPin);
    vCell1Array[i] = analogRead (voltPin1);
    vCell2Array[i] = analogRead(voltPin2);
    vCell3Array[i] = analogRead(voltPin3);
  } 
}

void StateManager::update() {
	updateCapacityConsumed();
  sampleSensors();
  getTemps();
}

void StateManager::updateCapacityConsumed() {			//updates capacity consumed
  static unsigned long localTimeKeeper = millis();
  unsigned long loopTime = (unsigned long)(millis() - localTimeKeeper);
  localTimeKeeper = millis();
  capacityConsumed += (current*(float)loopTime*0.001);          //amp seconds
}

void StateManager::sampleSensors() {			//smooths samples
	static unsigned int vTotal1 = 0;
  static unsigned int vTotal2 = 0;
  static unsigned int vTotal3 = 0;
  static unsigned int iTotal = 0;
  static int readindex = 0;
  static bool firstRun = true;

  if (firstRun == true) //runs only once
  {
    for (int i = 0; i < arraySize; i++) //initialise sums
    {
      vTotal1 = vTotal1 + vCell1Array[i];   //ok because volts won't go negative
      vTotal2 = vTotal2 + vCell2Array[i];
      vTotal3 = vTotal3 + vCell3Array[i];
      iTotal  = iTotal + currentArray[i];
    }
    firstRun = false;
  }
  vTotal1 = vTotal1 - vCell1Array[readindex];     //increase lower bound for the moving average
  vTotal2 = vTotal2 - vCell2Array [readindex];
  vTotal3 = vTotal3 - vCell3Array [readindex];
  iTotal  = iTotal  - currentArray[readindex];

  vCell1Array[readindex] =  analogRead (voltPin1);  // increase upperbound for moving average
  vCell2Array[readindex] =  analogRead (voltPin2);
  vCell3Array [readindex] = analogRead (voltPin3);
  currentArray[readindex] = analogRead (milliampPin);

  vTotal1 = vTotal1 + vCell1Array[readindex];   //add on upper bound
  vTotal2 = vTotal2 + vCell2Array[readindex];
  vTotal3 = vTotal3 + vCell3Array[readindex];
  iTotal = iTotal + currentArray[readindex];

   readindex++;
   
  if (readindex >= arraySize)
  {
    readindex = 0;
  }

  float avgCurrent  = iTotal  / arraySize; //current smoothing
  float avgVCell1   = vTotal1 / arraySize; //voltage Cell 1 smoothing
  float avgVCell2   = vTotal2 / arraySize; //voltage Cell2 smoothing
  float avgVCell3   = vTotal3 / arraySize; //votlage Cell3 smoothing

  //MEASURING CURRENT
  current = 0.0097752 * avgCurrent - 5;

  //MEASURING VOLTAGES: V1, V2, V3
  v1 = 0.004884 * avgVCell1;
  v2 = 0.008211 * avgVCell2;
  v3 = 0.012317 * avgVCell3;

  v3 = v3 - v2;
  v2 = v2 - v1;

}

void StateManager::getTemps() {
	sensors.requestTemperatures();
  lzoneTemps[0] = (long)(sensors.getTempC(zone1)*100);    //convert float to long
  lzoneTemps[1] = (long)(sensors.getTempC(zone2)*100);    // mutiplied by 100 to keep res
  lzoneTemps[2] = (long)(sensors.getTempC(zone3)*100);
  zoneTemps[0] = (unsigned long)(lzoneTemps[0]);
  zoneTemps[1] = (unsigned long)(lzoneTemps[1]);
  zoneTemps[2] = (unsigned long)(lzoneTemps[2]);    //is temperature * 100
}

	unsigned long StateManager::get_zoneTemps(int index) {
		return zoneTemps[index];
	}
	float StateManager::get_capacityConsumed() {
		return capacityConsumed;			//gives float
	}
	float StateManager::get_current() {
		return current;
	}
	float StateManager::get_v1() {
		return v1;
	}
	float StateManager::get_v2() {
		return v2;
	}
	float StateManager::get_v3() {
		return v3;
	}
	float StateManager::get_batteryPercent() {
		return (batteryCapacity - capacityConsumed)/batteryCapacity;
	}
	unsigned long StateManager::get_TempAverage() {
		return (zoneTemps[0] + zoneTemps[1] + zoneTemps[2])/3;
	}
	float StateManager::get_batteryCapacity() {
		return batteryCapacity;
	}
void StateManager::reset_capacityConsumed() {
	capacityConsumed = 0;
}