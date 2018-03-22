#include <Arduino.h>
#include <EEPROM.h>
#include <EepromManager1_1.h>
#include <Comms1_2.h>
#include <Settings.h>
#include <AltSoftSerial.h>

SerialCommsClass<HardwareSerial> HS ("Strings", 100);
EepromManager BattCapacity(511,1023,4);

typedef unsigned int tStamp;
tStamp savePeriod = 100;
tStamp timeStamp = 0;
unsigned long Capacity = 6553500;
bool pause = true;
void setup() {
  HS.begin(2000000);
  BattCapacity.memoryStatus(savePeriod);
}

void loop() {
  if (!pause) {
    if ((tStamp)((tStamp)millis() - timeStamp) >= savePeriod) {
      BattCapacity.write(Capacity++);
      Serial.println('.');
      timeStamp = (tStamp)millis();
    }
  }
  if (HS.read()) {
    switch (HS.get_opcode()) {
      case 'p':    //print memory contents and then pauses
        BattCapacity.printRange();
        pause = true;
        break;
      case 'q':    //toggle pause
        pause = !pause;
        break;
      case 'r':     //single step read
        BattCapacity.printVariables();
        Serial.print("Read data: ");
        Serial.println(BattCapacity.read());
        BattCapacity.printVariables();
        Serial.println("");
        pause = true;
        break;
      case 'w':      //single step write
        BattCapacity.printVariables();
        Capacity = *HS.get_operand(0);
        BattCapacity.write(Capacity);    //use first operand
        BattCapacity.printVariables();
        Serial.println("");
        pause = true;
        break;
      case 'd':
        savePeriod = *HS.get_operand(0);
        break;
       default: 
        break;
    }
  }
}
