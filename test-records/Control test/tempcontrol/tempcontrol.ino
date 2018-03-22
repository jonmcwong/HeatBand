#include <Arduino.h>
#include <Comms1_2.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <AltSoftSerial.h>
const int Heater1 = 11;
const int Heater2 = 5;
const int Heater3 = A5;
const int tempBusPin = 2;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(tempBusPin);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress zone1, zone2, zone3;
#define TEMPERATURE_PRECISION 9

SerialCommsClass<AltSoftSerial> BT ("Strings", 100);
SerialCommsClass<HardwareSerial> HS ("Strings", 100);
AltSoftSerial TEST;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Setup
//temp variables
float tempData[3];
unsigned long utempData[3];
long ltempData[3];
bool standBy = true;
unsigned long zeros[3] = {6546,1,4563};

//timing variables
unsigned long tStamp;

void setup() {
  // put your setup code here, to run once:
  BT.begin(9600);
    HS.begin(9600);
    pinMode(tempBusPin,INPUT);
  sensorSetup();
  pinMode(Heater1, OUTPUT);
  pinMode(Heater2, OUTPUT);
  pinMode(Heater3, OUTPUT);
  digitalWrite(Heater1, LOW);
  digitalWrite(Heater2, LOW);
  digitalWrite(Heater3, LOW);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
 pinMode(A4,INPUT);



}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////loop
void loop() {
  // put your main code here, to run repeatedly:


  if (BT.read()) {
    Serial.println("...........");
    if (BT.get_opcode() == 'p') {
      standBy = !standBy;
      TEST.write('\n');
      Serial.print(BT.get_opcode());
    }
  } //end of BT


  if (millis() - tStamp > 500) {    //if half a second [ast
    tStamp = millis();
      Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();      //get temps
  Serial.println("DONE");
  tempData[0] = sensors.getTempC(zone1);
  tempData[1] = sensors.getTempC(zone2);
  tempData[2] = sensors.getTempC(zone3);
  ltempData[0] = (long)(tempData[0]*100);
  ltempData[1] = (long)(tempData[1]*100);
  ltempData[2] = (long)(tempData[2]*100);
  utempData[0] = (unsigned long)ltempData[0];
  utempData[1] = (unsigned long)ltempData[1];
  utempData[2] = (unsigned long)ltempData[2];
  // print the device information
  BT.write('g', utempData );
  TEST.write('\n');
  printData(zone1);
  printData(zone2);
  printData(zone3);
    if (!standBy) {
        //for zone1
      if (tempData[0] < 35) {
        digitalWrite(Heater1, HIGH);
      } else {
        digitalWrite(Heater1, LOW);
      } // end of zone 1 control
          //for zone2 
      if (tempData[1] < 28) {
        digitalWrite(Heater2, HIGH);
      } else {
        digitalWrite(Heater2, LOW);
      } // end of zone 2 control
    
    
          //for zone3
      if (tempData[2] < 21) {
        digitalWrite(Heater3, HIGH);
      } else {
        digitalWrite(Heater3, LOW);
      } // end of zone 3 control
    } else {
      digitalWrite(Heater1, LOW);
      digitalWrite(Heater2, LOW);
      digitalWrite(Heater3, LOW);
    
    }   //end of standby check 
  } //end of non blocking delay
  
  

} //end of loop
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////end of loop
//Temperature function
// function to print a device address
void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress){
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}


void sensorSetup() {

  // Start up the library
  sensors.begin();  

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // method 1: by index
  if (!sensors.getAddress(zone1, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(zone2, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(zone3, 2)) Serial.println("Unable to find address for Device 2");
  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to zone1
  //if (!oneWire.search(zone1)) Serial.println("Unable to find address for zone1");
  // assigns the seconds address found to zone2
  //if (!oneWire.search(zone2)) Serial.println("Unable to find address for zone2");

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
}   //end of setup sensors

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}
