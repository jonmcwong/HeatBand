#include <Arduino.h>
#include <EEPROM.h>
#include <EepromManager1_1.h>
#include <Comms1_2.h>
#include <Settings.h>
#include <AltSoftSerial.h>


SerialCommsClass<AltSoftSerial> BT ("Strings", 100);
//Settings already instantiated
EepromManager SettingsMemory(0,9,4);    //used purely for the print function
unsigned long data[3];
void setup() {
  // put your setup code here, to run once:
  BT.begin(9600);
  Serial.begin(9600);
  Settings.begin(); //sets last setting
  Settings.memoryStatus();
  
}

void loop() {
  //Serial.println("....................................");
  if (BT.read()) {
    switch (BT.get_opcode()) {
      case 'n':   //Store Settings
        Settings.storeSettingOfMode (1, BT.get_operand());
        SettingsMemory.printRange();
        break;
      case 'o':   //Store Settings
        Settings.storeSettingOfMode (2, BT.get_operand());
        SettingsMemory.printRange();
        break;
      case 'p':   //Store Settings
        Settings.storeSettingOfMode (3, BT.get_operand());
        SettingsMemory.printRange();
        break;
      case 'a':   //getSettings
        Settings.getSettingOfMode(1, data);
        BT.write('a', data);
        SettingsMemory.printRange();
        break;
      case 'b':   //getSettings
        Settings.getSettingOfMode(2, data);
        BT.write('b', data);
        SettingsMemory.printRange();
        break;
      case 'c':  //getSettings
        Settings.getSettingOfMode(3, data);
        BT.write('c', data);
        SettingsMemory.printRange();
        break;
      case 'q':   //write Settings
        Settings.writeCurrentMode(*BT.get_operand(0));
        Settings.printVariables();
        break;
      case 'y':     //for cycle mode
        Settings.printVariables();
        Serial.println(F("CycleMode"));
        Settings.cycleMode();
        Settings.printVariables();
        break;
      default:
        break;
    }
  }
}
