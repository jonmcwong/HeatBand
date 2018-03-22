//Library containing all the functions that help the band and the 
//android app interface with the Settings
//Instantiation already made called "Settings"

//Last used mode is stored at 0 in EEPROM
//each zone is 1 byte
//mode 1 is at address 2 
//mode 2 is at address 5
//mode 3 is at address 8



#ifndef Settings_h
#define Settings_h

#ifndef NO_OF_MODES
#define NO_OF_MODES 3
#endif

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "EEPROM.h"

class SettingsClass {
public:
	//PRIMARY METHODS
		SettingsClass (const unsigned int& rootAddress);
		void memoryStatus();	//prints out the range of memory used.
		bool storeSettingOfMode (const byte& mode, const unsigned long data[3]);	//response to 
		                                  //Bluetooth command. Updates the relevant 
		                                  //mode on the EEPROM with given temperatures
		                                  //in array. They must all be between
		                                  //0 - 255. returns true on success. But 
		                                  //false if data is invalid  
		bool getSettingOfMode (const byte& mode, unsigned long (&data)[3]);	//returns data for given mode from EEPROM

	//SECONDARY METHODS, USE PRIMARY
		void cycleMode();									//Sets all the settings to that of the 
		                                  //next mode
		bool writeCurrentMode(const byte& mode);  //input mode is used to set currentMode
		                                  //and retrieve currentMode settings from EEPROM
		                                  //can be used upon power up to switch to the
		                                  //last mode, can be used for updating current mode. Updates last mode
	//TERTIARY METHODS, USE PRIMARY AND SECONDARY
		void begin();		 //used on initialisation. gets last mode and sets current mode

	//ACCESS METHODS
		void printVariables();
		int get_currentMode();
		int get_currentZone1();
		int get_currentZone2();
		int get_currentZone3();
private:
	int _currentMode;
	int _currentZone1;
	int _currentZone2;
	int _currentZone3;
	const int _Root_Address;		//Holds the address of the EEPROM
                              //that stores the last saved mode
};
static SettingsClass Settings((unsigned int)0); ///deal with later
#endif