#include <Arduino.h>
#include <EepromManager1_1.h>
#include <Comms1_2.h>
#include <Settings.h>



SerialCommsClass<HardwareSerial> HS ("Strings", 100);
//Settings already instantiated
EepromManager SettingsMemory(0,9,4);    //used purely for the print function
unsigned long data[3];
void setup() {
  // put your setup code here, to run once:
  HS.begin(2000000);
  Settings.begin(); //sets last setting
  Settings.memoryStatus();
  
}

void loop() {
  if (HS.read()) {
    switch (HS.get_opcode()) {
      case 'n':   //Store Settings
        Settings.storeSettingsOfMode (1, HS.get_operand());
        SettingsMemory.printRange();
        break;
      case 'o':   //Store Settings
        Settings.storeSettingsOfMode (2, HS.get_operand());
        SettingsMemory.printRange();
        break;
      case 'p':   //Store Settings
        Settings.storeSettingsOfMode (3, HS.get_operand());
        SettingsMemory.printRange();
        break;
      case 'a':   //getSettings
        Settings.getSettingOfMode(1, data);
        HS.write('a', data);
        SettingsMemory.printRange();
        break;
      case 'b':   //getSettings
        Settings.getSettingOfMode(2, data);
        HS.write('b', data);
        SettingsMemory.printRange();
        break;
      case 'c':  //getSettings
        Settings.getSettingOfMode(3, data);
        HS.write('c', data);
        SettingsMemory.printRange();
        break;
      case 'q':   //write Settings
        Settings.writeCurrentMode(*HS.get_operand(0));
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
