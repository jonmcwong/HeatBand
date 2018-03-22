#include <Arduino.h>
#include <EEPROM.h>
#include <EepromManager.h>
#include <Comms1_2.h>
#include <Settings.h>
#include <AltSoftSerial.h>

SerialCommsClass<AltSoftSerial> BT ("Bytes", 100);             //Bluetooth
SerialCommsClass<HardwareSerial> HS ("Strings", 100);

void setup() {
  HS.begin(9600);
  BT.begin(9600);
}

void loop() {

  if (BT.read()) {
    HS.write(BT.get_opcode(), BT.get_operand());
    BT.write(BT.get_opcode(), BT.get_operand());
  }
}
