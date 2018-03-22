#include <Arduino.h>
#include <EepromManager1_1.h>
#include <CommsTest.h>
#include <Settings.h>
#include <AltSoftSerial.h>

/*Contents:
-Global Variables
-test Variables
-Pin Declarations
-setup()
  -Pin Setup
  -Begins
-Loop()
-Functions


*/

//GLOBAL VARIABLES
SerialCommsClass<HardwareSerial> HS ("Strings", 100); //Hardware Serial Comms
SerialCommsClass<AltSoftSerial> BT ("Strings", 100);  //Bluetooth Serial Comms

EepromManager BattCapacity (10, 1023, 3);             //WearLevelling range, stampsize
unsigned long mainTimeKeeper = 0; //holds main program time
int batteryCapacity = 1000; //battery capacity in mA, the calculations allow for a 20% charge being left in the battery
float capacityConsumed = 0;
//declaring sizes for arrays
const byte arraySize = 30; //nominal size 30

//variables for measuring current
unsigned int currentArray[arraySize];
float current = 0;      //final current value

//VARIABLES for measuring voltage
unsigned int vCell1Array[arraySize];
unsigned int vCell2Array[arraySize];
unsigned int vCell3Array[arraySize];
float v1 = 0;   //final voltage values
float v2 = 0;
float v3 = 0;

//test Variables
bool pause = true;
unsigned int tStamp;

//PIN DECLARATIONS
//const byte = 0;
//const byte = 1;
const byte tempBusPin = 2;
const byte cycleModeBtnPin = 3;
const byte mosfetPinCell1 = 4;
const byte heaterSwitchPin2 = 5;
//const byte = 6;
const byte mosfetPinCell2 = 7;
//const byte = 8;   //AltSoftSerial RX
//const byte = 9;   //AltSoftSerial TX
const byte mosfetPinCell3 = 10;
const byte heaterSwitchPin1 = 11;
const byte redLedPin = 12;     //was 3 in Dimi's code
const byte greenLedPin = 13;
const byte voltPin1= 14;    //A0
const byte voltPin2 = 15;    //A1
//const byte  = 16;    //A2
const byte heaterSwitchPin3 = 17;    //A3
const byte status1 = 18;    //A4
const byte status2 = 19;    //A5
const byte milliampPin = 20;    //A6
const byte voltPin3 = 21;    //A7

  unsigned long stuff[3] = {0,0,0};


void setup() {
  //PIN SETUP
  
  pinMode(cycleModeBtnPin, INPUT);
  pinMode(mosfetPinCell1, OUTPUT);
  pinMode(heaterSwitchPin2, OUTPUT);
  pinMode(mosfetPinCell2, OUTPUT);      //ERROR HERE
  pinMode(mosfetPinCell3, OUTPUT);      
  
  pinMode(heaterSwitchPin1, OUTPUT);
  
  pinMode(redLedPin, OUTPUT); 
  pinMode(greenLedPin, OUTPUT);
  pinMode(voltPin1, INPUT);             //A0
  pinMode(voltPin2, INPUT);             //A1
  pinMode(heaterSwitchPin3, OUTPUT);    //A3
  pinMode(status1, INPUT);              //A4
  pinMode(status2, INPUT);              //A5
  pinMode(milliampPin, INPUT);          //A6
  pinMode(voltPin3, INPUT);             //A7

  //BEGINS

  HS.begin(9600);
  BT.begin(9600);
//  BattCapacity.begin();
  Settings.begin();

  BT.write('a', stuff);
  
  digitalWrite(heaterSwitchPin1, LOW);
  digitalWrite(heaterSwitchPin2, LOW);
  digitalWrite(heaterSwitchPin3, LOW);

  void initialiseArrays ();
}


//void batteryLedBlink (unsigned long mainTimeKeeper,int mode,int capacityConsumed,int batteryCapacity);
//void lightLeds (int capacityConsumed, int batteryCapacity);


void loop () {
  //Serial.println(BT.available());
      static bool check = true;
  mainTimeKeeper = millis();  

  batteryLedBlink(mainTimeKeeper, Settings.get_currentMode(), capacityConsumed, batteryCapacity);
  //BT.write('c', stuff);
  sampleSensors();    //should update sensor global variables
  updateCapacityConsumed(mainTimeKeeper);
  
  if (BT.read()) {

    
    switch (BT.get_opcode()) {
      case 'l':     //for changing capacity
        capacityConsumed = *BT.get_operand(0);
        Serial.println(capacityConsumed);
        break;
      case 'q':   //write Settings
        Settings.writeCurrentMode(*BT.get_operand(0));
        Settings.printVariables();
        break;
      case 'p':    //toggle pause
        pause = !pause;
        break; 
      default:
        break;
    }
  }
  if (!pause) {
    if (check || ((unsigned int)(unsigned int)millis()- tStamp) >= 500) {
      
      unsigned long vDataArray[3] = {(unsigned long)(v1 * 1000), (unsigned long)(v2 * 1000), (unsigned long)(v3 * 1000)};
      BT.write('a', vDataArray);
          Serial.print(v1);
          Serial.print(", ");
          Serial.print(v2);
          Serial.print(", ");
          Serial.println(v3);
          printArray (vCell1Array);printArray (vCell2Array);printArray (vCell3Array);

       check = false;
       tStamp = (unsigned int)millis();
       
    }

  }
}

void batteryLedBlink (unsigned long mainTimeKeeper,int mode,int capacityConsumed,int batteryCapacity){
  //TESTED AND WORKING

//blinks leds 1 time for mode 1, 2 times for mode 2 and 3 times for mode 3, 

   static unsigned long primaryBlinkRate = 2000;
   static unsigned long secondaryBlinkRate = 150;
   static int numberOfBlinks = 0;
   static unsigned long primaryLocalTimeKeeper = 0;
   static unsigned long secondaryLocalTimeKeeper = 0;
   static bool toggle1 = false;
   static bool toggle2 = false;
   
  if ( (mainTimeKeeper - primaryLocalTimeKeeper) >= primaryBlinkRate)  //toggle for main blinking rate
  {
    primaryLocalTimeKeeper = millis();    //stops ot from being called within a 2 second time period
    toggle1 = !toggle1;                   //toggle1 is true when blinking
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
    lightLeds(capacityConsumed,batteryCapacity);
  }
  else
  {
    //digitalWrite(13,LOW);
    digitalWrite(redLedPin,LOW);
    digitalWrite(greenLedPin,LOW);
  }
 
}

void lightLeds (int capacityConsumed, int batteryCapacity){//TESTED AND WORKING 
//lights appropriate led colour by reading capacity consumed

  if (current > 0) {
    if (capacityConsumed > 0.6 * batteryCapacity)
    {
      digitalWrite(redLedPin,HIGH); //red
      digitalWrite(greenLedPin,LOW);
    }
    else 
    {
      digitalWrite(redLedPin,LOW);
      digitalWrite(greenLedPin,HIGH); //green
    }
  } else {
        digitalWrite(redLedPin,HIGH);
    digitalWrite(greenLedPin,HIGH);
  }
}

int updateCapacityConsumed (unsigned long mainTimeKeeper){//updates global battery capacity remaining

  static unsigned loopTime = 0;
  static unsigned localTimeKeeper = (unsigned)millis();
  
  
  loopTime = (unsigned)((unsigned)millis() - localTimeKeeper);
  localTimeKeeper = (unsigned)millis();
  capacityConsumed = capacityConsumed + (current*(float)loopTime*0.001);
  return capacityConsumed;
} 
//***SMOOTHING ARRAY INITIALISATION****


void initialiseArrays () {
  for (int i = 0; i < arraySize; i++)
  {
    currentArray[i] = analogRead (milliampPin); //analogRead(milliampPin);
    vCell1Array[i] = analogRead (voltPin1);
    vCell2Array[i] = analogRead(voltPin2);
    vCell3Array[i] = analogRead(voltPin3);
  }
}

//updates global variables +smoothening
void sampleSensors() {
  static long vTotal1 = 0;
  static long vTotal2 = 0;
  static long vTotal3 = 0;
  static long iTotal = 0;
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

  vTotal1 = vTotal1 - vCell1Array[readindex];
  vTotal2 = vTotal2 - vCell2Array [readindex];
  vTotal3 = vTotal3 - vCell3Array [readindex];
  iTotal  = iTotal  - currentArray[readindex];


  vCell1Array[readindex] =  analogRead (voltPin1);
  vCell2Array[readindex] =  analogRead (voltPin2);
  vCell3Array [readindex] = analogRead (voltPin3);
  currentArray[readindex] = analogRead (milliampPin);
  /*
    Serial.println("vcell1: ");
    printArray(vCell1Array);
    Serial.println("vcell2: ");
    printArray(vCell2Array);
    Serial.println("vcell3: ");
    printArray(vCell3Array);
    Serial.println("currentArray: ");
    printArray(currentArray);
  */


  vTotal1 = vTotal1 + vCell1Array[readindex];
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
  
/*
  Serial.print("Average current: ");
  Serial.println(avgCurrent);
  Serial.print("Average v1: ");
  Serial.println(avgVCell1);
  Serial.print("Average v2: ");
  Serial.println(avgVCell2);
  Serial.print("Average v3: ");
  Serial.println(avgVCell3);

*/
  //MEASURING CURRENT
  
  current = 0.0097752 * avgCurrent - 5;

  //MEASURING VOLTAGES: V1, V2, V3
  v1 = 0.004884 * avgVCell1;
  v2 = 0.008211 * avgVCell2;
  v3 = 0.012317 * avgVCell3;


  v3 = v3 - v2;
  v2 = v2 - v1;

  
  /*
  Serial.println("*****************************************************");
  Serial.print("V3: ");
  Serial.println(v3);
   Serial.print("V2: ");
  Serial.println(v2);
    Serial.print("V1: ");
  Serial.println(v1);
    Serial.print("Current: ");
  Serial.println(current); */
}

void printArray (unsigned int Array[]) {
  for (int i = 0; i < arraySize; i++) {
    Serial.print(Array[i]);
    Serial.print(',');
  }
  Serial.println("");
}






/*
//commands returns false for invalid input, true otherwise
  bool transmitMode1() {    //a
    byte* data = Settings.getSettingOfMode(1);  //gets the array of data
    return Comms.write('a', data[0], data[1], data[2]);   //convention is zone1, zone 2 zone 3
  }         

  bool transmitMode2() {    //b
    byte* data = Settings.getSettingOfMode(2);  //gets the array of data
    return Comms.write('b', data[0], data[1], data[2]);
  }       
  bool transmitMode3(){         //c
    byte* data = Settings.getSettingOfMode(3);  //gets the array of data
    return Comms.write('c', data[0], data[1], data[2]);
  }     

  bool transmitCurMode(){      //d
    return Comms.write('d', 0, 0, Settings.get_currentMode());
  }    

  bool confirmReceive(){          //e
    return Comms.write('e', 0, 0, (byte)opcode);

  }  
  bool transmitBattPercent(){
    return Comms.write('f',);  //FILL IN WITH DIMI'S FUNCTION

  }   //f
  bool transmitAvTemp(){      //g
    return Comms.write('g',);    //FILL IN WITH AVERAGE TEMPERAURE

  }      
  bool switchOff(){     //h
    return Comms.write('h', 0, 0,0);

  }           
  bool switchOn(){        //i
    return Comms.write('i', 0, 0 ,0xFF );

  }          
  bool transmitLowBatt(){    //j
    return Comms.write('j', )     //FILL IN WITH BATTERY PERCENT

  }      

  */
