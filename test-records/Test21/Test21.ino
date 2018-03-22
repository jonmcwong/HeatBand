#include <OneWire.h>
#include <DallasTemperature.h>
#include <CommsTest.h>
#define tempBusPin 2
#define TEMPERATURE_PRECISION 9

SerialCommsClass<AltSoftSerial> BT ("Strings", 100);
OneWire oneWire(tempBusPin);
DallasTemperature sensors(&oneWire);
DeviceAddress zone1, zone2, zone3;

bool pause = false;
bool heatersState = false;
unsigned int tStamp;
const byte heater1 = 5;
const byte heater2 = 11;
const byte heater3 = 17;

void setup() {

  // put your setup code here, to run once:
  pinMode(heater1, OUTPUT);
  pinMode(heater2, OUTPUT);
  pinMode(heater3, OUTPUT);
  sensors.begin();
  Serial.begin(9600);
  //Find and locate sensors
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  //Set parasite power
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  //Get Sensor index
  if (!sensors.getAddress(zone1, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(zone2, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(zone3, 2)) Serial.println("Unable to find address for Device 2");
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(zone1);
  Serial.println();
  Serial.print("Device 1 Address: ");
  printAddress(zone2);
  Serial.println();
  Serial.print("Device 1 Address: ");
  printAddress(zone3);
  Serial.println();
  // set the resolution to 9 bit per device
  sensors.setResolution(zone1, TEMPERATURE_PRECISION);
  sensors.setResolution(zone2, TEMPERATURE_PRECISION);
  sensors.setResolution(zone3, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(zone1), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(zone2), DEC);
  Serial.println();

  Serial.print("Device 2 Resolution: ");
  Serial.print(sensors.getResolution(zone3), DEC);
  Serial.println();
}


void loop() {

  if (BT.read()) {
    switch (BT.get_opcode()) {
      case 'p':    //toggle pause
        pause = !pause;
        break;
      case 'h':     //heaters toggle
        heatersState = !heatersState;
      default:
        break;
    }
  }

  digitalWrite(heater1, heatersState);
  digitalWrite(heater2, heatersState);
  digitalWrite(heater3, heatersState);
  
    
    if (!pause) {
    static bool check = true;
  
    if (check || ((unsigned int)(unsigned int)millis()- tStamp) >= 500) {
      sensors.requestTemperatures();
      unsigned long data[3] = {(unsigned long)getTempInC(zone1), (unsigned long)getTempInC(zone2), (unsigned long)getTempInC(zone3)};
      BT.write('\n', data);
      check = false;
      tStamp = (unsigned int)millis();
    }
  // put your main code here, to run repeatedly:
}
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

unsigned int getTemperature(DeviceAddress deviceAddress)
{
  unsigned int tempC = round(map(sensors.getTempC(deviceAddress),-10,60,0,65536));
  Serial.print("Temp C: ");
  Serial.print(tempC);
  
  return tempC;    //returns temp in range -10 to 60 as integer linearly mapped to range 0 to 65536
}

float getTempInC(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  return tempC;
}


