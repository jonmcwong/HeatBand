//Library containing all the functions that help the band and the 
//android app interface with the Settings

#include "Settings.h"

SettingsClass::SettingsClass (const unsigned int& rootAddress)
: _Root_Address (rootAddress)			//constructor, root address holds the lowest address number
																	//that is used for settings
{}

void SettingsClass::memoryStatus() {
	Serial.flush();
	Serial.print(F("Settings uses "));
	Serial.print(NO_OF_MODES * 3 + 1);			//used to be Serial.print(noOfModes)
	Serial.print(F(" bytes, from address "));
	Serial.print(_Root_Address);
	Serial.print(F(" to "));
	Serial.println(_Root_Address + NO_OF_MODES * 3);			//used to be Serial.println(rootAddress + 3 * noOfModes);
	Serial.flush();
}

bool SettingsClass::storeSettingOfMode (const byte& mode, const unsigned long data[3]) {

	//response to 
  //Bluetooth command. Updates the relevant 
  //mode on the EEPROM with given temperatures
  //in array. They must all be between
  //0 - 255. returns true on success. But 
  //false if data is invalid
	 if (mode > NO_OF_MODES || mode < 1) {
  	Serial.println(F("Mode Out of Range"));
  	Serial.flush();
  	return false;
  }
EEPROM.update(_Root_Address + (mode - 1) * 3 + 1, (byte)data[0]);
EEPROM.update(_Root_Address + (mode - 1) * 3 + 2, (byte)data[1]);
EEPROM.update(_Root_Address + (mode - 1) * 3 + 3, (byte)data[2]);
return true;
}	

bool SettingsClass::getSettingOfMode (const byte& mode, unsigned long (&data)[3]) {
	//returns data for given mode from EEPROM
	 if (mode > NO_OF_MODES || mode < 1) {
  	Serial.println(F("Mode Out of Range"));
  	Serial.flush();
  	return false;
  }
	data[0] = (unsigned long)EEPROM.read(_Root_Address + (mode - 1) * 3 + 1);
	data[1] = (unsigned long)EEPROM.read(_Root_Address + (mode - 1) * 3 + 2);
	data[2] = (unsigned long)EEPROM.read(_Root_Address + (mode - 1) * 3 + 3);
	return true;
}
bool SettingsClass::writeCurrentMode (const byte& mode) {
	//input mode is used to set currentMode
  //and retrieve currentMode settings from EEPROM
  //can be used upon power up to switch to the
  //last mode, can be used for updating current mode. Updates last mode
  //returns false if mode is invalid and true otherwise
  byte getMode = mode;
  if (mode > NO_OF_MODES || mode < 1) {
  	getMode = 1;
  	Serial.println(F("Mode out of range. Set to default"));
  }
	unsigned long retreivedData[3]; 		//one for each zone not mode
	getSettingOfMode(getMode, retreivedData);
	_currentMode = getMode;
	_currentZone1 = (byte)retreivedData[0];
	_currentZone2 = (byte)retreivedData[1];
	_currentZone3 = (byte)retreivedData[2];
	EEPROM.update(_Root_Address, getMode);		//remembers last saved mode
	return true;
}

void SettingsClass::cycleMode() {
	writeCurrentMode(_currentMode % NO_OF_MODES + 1 );  //used to be writeCurrentMode(_currentMode % noOfModes + 1 );
}

void SettingsClass::printVariables() {
	Serial.print(F("Current Mode: "));
	Serial.println(_currentMode);
	Serial.print(F("Zone 1: "));
	Serial.println(_currentZone1);
	Serial.print(F("Zone 2: "));
	Serial.println(_currentZone2);
	Serial.print(F("Zone 3: "));
	Serial.println(_currentZone3);
	Serial.println();
}

void SettingsClass::begin() {
	writeCurrentMode(EEPROM.read(_Root_Address));
}

int SettingsClass::get_currentMode() {
	return _currentMode;
}

int SettingsClass::get_currentZone1() {
	return _currentZone1;
}

int SettingsClass::get_currentZone2() {
	return _currentZone2;
}

int SettingsClass::get_currentZone3() {
	return _currentZone3;		
}