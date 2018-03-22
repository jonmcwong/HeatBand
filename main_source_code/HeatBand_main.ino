#include <EepromManager1_1.h>
#include <Comms1_2.h>
#include <Settings.h>
#include <State.h>
#include <Button.h>
#include <LED.h>

//TYPEDEFs
typedef unsigned long ms_time;       //specific for timing
typedef unsigned long tempData;

//PIN DECLARATIONS (ALIASES)
//const byte = 0;
//const byte = 1;
const byte tempBusPin = 2;
const byte cycleModeBtnPin = 3;
const byte mosfetPinCell1 = 4;
//const byte = 6;                   //unused
const byte mosfetPinCell2 = 7;
//const byte = 8;                   //AltSoftSerial RX
//const byte = 9;                   //AltSoftSerial TX
const byte mosfetPinCell3 = 10;
const byte redLedPin = 12;          //was 3 in Dimi's code
const byte greenLedPin = 13;
const byte voltPin1= 14;            //A0
const byte voltPin2 = 15;           //A1
//const byte  = 16;                 //A2
//const byte  = 17;                 //A3
//const byte  = 18;                 //A4
const byte milliampPin = 20;        //A6
const byte voltPin3 = 21;           //A7

const int Heater1 = 11;
const int Heater2 = 5;
const int Heater3 = A5;


//GLOBAL VARIABLES

//OBJECTS
SerialCommsClass<AltSoftSerial> BT ("Strings", 100);  //For Bluetooth Serial Comms
EepromManager BattCapacity (10, 1023, 3);             //For storing data to Eeprom with WearLevelling
Button myBtn(cycleModeBtnPin, true, false, 100);      //For debouncing the button
                                                      //pin number, internal pullup, inverted, dbtime
StateManager StateTracker (voltPin1, voltPin2, voltPin3, milliampPin, tempBusPin); 
                                                      //For measuring of all the relevant states
LED myLed(1000,100,redLedPin, greenLedPin);           //For controlling the Led blink

//VARIABLES for responding to commands
tempData tempArray[3];

//VARIABLES for time keeping
static ms_time mainTimeKeeper = 0;

//VARIABLES for temp control
bool heatersOn = false;

//VARIABLES for Timer
ms_time cutOff;
bool timerOn = false;

//VARIABLES FOR battery management
const float cellDangerThreshold = 4.2;
ms_time bmsTriggerTime = 500;
ms_time savePeriod = 5000;

void setup() {
//PIN SETUP
pinMode(cycleModeBtnPin, INPUT_PULLUP);
pinMode(mosfetPinCell1, OUTPUT);
pinMode(mosfetPinCell2, OUTPUT);    
pinMode(mosfetPinCell3, OUTPUT);
pinMode(redLedPin, OUTPUT);
pinMode(greenLedPin, OUTPUT);
pinMode(voltPin1, INPUT);             //A0
pinMode(voltPin2, INPUT);             //A1
pinMode(A2, INPUT);                   //A2
pinMode(A3, INPUT);                   //A3
pinMode(A4, INPUT);                   //A4
pinMode(milliampPin, INPUT);          //A6
pinMode(voltPin3, INPUT);             //A7

pinMode(Heater1, OUTPUT);
pinMode(Heater2, OUTPUT);
pinMode(Heater3, OUTPUT);
//set initial setting
digitalWrite(Heater1, LOW);
digitalWrite(Heater2, LOW);
digitalWrite(Heater3, LOW);

//BEGINS
BT.begin(9600);                       //Sets baud rate for AltSoftSerial
BattCapacity.begin();                 //initialises memory from Eeprom
Settings.begin();                     //initialises memory from Eeprom
StateTracker.begin(BattCapacity.read());      //Sets up sensors and moving averages
}




void loop () {                          //Continuously repeat
  if (BT.read()) {                      //Listen out for commands
    switch (BT.get_opcode()) {          //all band receive commands
      case 'm':                         //Send Setting data
        Settings.getSettingOfMode(1, tempArray);  //gets setting data
        BT.write('a',tempArray);                  //transmit setting data
        Settings.getSettingOfMode(2, tempArray);  //gets setting data
        BT.write('b',tempArray);                  //transmit setting data
        Settings.getSettingOfMode(3, tempArray);  //gets setting data
        BT.write('c',tempArray);                  //transmit setting data
        break;
      case 'n':                         //change setting data
        Settings.storeSettingOfMode(1, BT.get_operand());
        break;
      case 'o':                         //change setting data
        Settings.storeSettingOfMode(2, BT.get_operand());
        break;
      case 'p':                         //change seeting data
        Settings.storeSettingOfMode(3, BT.get_operand());
        break;
      case 'q': {                       //set current mode
        if (*BT.get_operand() == 0) {
          heatersOn = false;
          break;
        } else {
          Settings.writeCurrentMode(*BT.get_operand());
          heatersOn = true;
        }
        break; }
      case 'r':                         //send battery percent
          tempArray[0] = (tempData)(StateTracker.get_batteryPercent());
          tempArray[1] = 0;
          tempArray[2] = 0;
          BT.write('f',tempArray);
        break;
      case 's':                         //send zone temperature
        tempArray[0] = (StateTracker.get_zoneTemps(0)/10 + 100);  //preagreed mapping
        tempArray[1] = (StateTracker.get_zoneTemps(1)/10 + 100);
        tempArray[2] = (StateTracker.get_zoneTemps(2)/10 + 100);
        BT.write('g', tempArray);
        break;
      case 'u':                         //set timer function
        if (*BT.get_operand(0) || *BT.get_operand(1)) {
          timerOn = true;
          cutOff = millis() + (*BT.get_operand(0)) * 3600000 + (*BT.get_operand(1)) * 60000;
          //current time + the hours and minutes in ms
        } else {
          timerOn = false;
        }
        break;
      default:
        break;
      }
    }
  
  //loop functions
  mainTimeKeeper = millis();
  StateTracker.update();      //update all sensors, collect all data
  myLed.blink(mainTimeKeeper, Settings.get_currentMode(), StateTracker.get_TempAverage(), 40);
  BMS();
  myBtn.read();

  //CONTROL LOOP
  //if heaters setting is On
  if (heatersOn) { 
      //control for zone 1
      if (StateTracker.get_zoneTemps(0) > (((unsigned long)Settings.get_currentZone1())*40/255 + 20)*100) { //is temp *100
        digitalWrite(Heater1, LOW);
      } else {
        digitalWrite(Heater1, HIGH);
      }
      //control for zone 2
      if (StateTracker.get_zoneTemps(1) > (((unsigned long)Settings.get_currentZone2())*40/255 + 20)*100) {
        digitalWrite(Heater2, LOW);
      } else {
        digitalWrite(Heater2, HIGH); 
      }
      //control loop for zone 3
      if (StateTracker.get_zoneTemps(2) > (((unsigned long)Settings.get_currentZone2())*40/255 + 20)*100) {
        digitalWrite(Heater3, LOW);
      } else {
        digitalWrite(Heater3, HIGH);
      }
  } else {    //If heater not on, switch off
      digitalWrite(Heater1, LOW);
      digitalWrite(Heater2, LOW);
      digitalWrite(Heater3, LOW);
  }
  //setting 100 percent battery value adjusting
  if (StateTracker.get_v1() >= cellDangerThreshold &&
    StateTracker.get_v2() >= cellDangerThreshold &&
    StateTracker.get_v3() >= cellDangerThreshold) {
    StateTracker.reset_capacityConsumed();      //if batteries are full, set capacity consumed to 0
  }
  //Cycle mode button
  if (myBtn.wasPressed()) {//cycle trough by 1 mode;
    Settings.cycleMode();
  }
  //timer setting
  if (timerOn && cutOff -  millis() < 0) {  //condition for time to be up
                                            //accounts for roll over as cast to 
                                            //unsigned int is equivalent to modulo
    heatersOn = false;
    timerOn = false;
  }

  //timing for sending current temperatures
  static ms_time tStamp = millis();
    if (millis() - tStamp > 500) {
      tStamp = millis();
    tempArray[0] = (StateTracker.get_zoneTemps(0)/10 + 100);
    tempArray[1] = (StateTracker.get_zoneTemps(1)/10 + 100);
    tempArray[2] = (StateTracker.get_zoneTemps(2)/10 + 100);
    BT.write('g', tempArray);
    }
  //Timing for saving battery
    static ms_time saveBattStamp = millis();
    if (millis() - saveBattStamp > savePeriod) {
      saveBattStamp = millis();
      BattCapacity.write((unsigned long)((long)StateTracker.get_capacityConsumed())*3);
    } //end of capacity save

}


void BMS() {//monitors cell voltages  and applies corrections
  static ms_time localTimeKeeper = 0; //holds local BMS time
  
  if (millis() - localTimeKeeper > bmsTriggerTime)
  {
    localTimeKeeper = millis();            
    if( StateTracker.get_v1() >= cellDangerThreshold)    //for cell 1
    {
      digitalWrite(mosfetPinCell1,LOW);
    }
    else
    {
      digitalWrite(mosfetPinCell1,HIGH);    //was both high high
    }
    
    if( StateTracker.get_v2() >= cellDangerThreshold)   //for cell 2
    {
      digitalWrite(mosfetPinCell2,LOW);
    }
    else
    {
      digitalWrite(mosfetPinCell2,HIGH);
    }

    if( StateTracker.get_v3() >= cellDangerThreshold)   //for cell 3
    {
      digitalWrite(mosfetPinCell3,LOW);
    }
    else
    {
      digitalWrite(mosfetPinCell3,HIGH);
    }
  }
}

  
