/*
EepromManager.h - Header containing all the relevant functions to do with 
Managing memory usage in the EEPROM when data needs to be written frequently.

-Class basically turns a defined range of memory into one single stamp that can 
be over written again and again.
*/


#ifndef EepromManager1_1_h
#define EepromManager1_1_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "EEPROM.h"



class EepromManager {

public:
	typedef int MemAddress;
	typedef unsigned long SaveData;
	EepromManager(MemAddress lowerBound, 	//constructor
								MemAddress upperBound, 
								int stampSize);
	void memoryStatus(const unsigned int& savePeriod); 	//serial prints the status of the
								  							//memory. Outlines structure
																//of each stamp and how many stamps can
																//be saved before iterator loops back round
																//and lifespan of EEPROM with current 
																//settings. Also explains how many bytes are 
																//unused at the bounds
  void write(const SaveData& data); //takes an unsigned long to be recorded,
  															//writes it to the appropriate address
  SaveData read(); 							//Returns last saved value either from the 
  															//EEPROM or _lastSavedData
  															//Like any other data type, accessing an 
  															//uninitialised EEPROM will return a random
  															//value
  void begin(); 	//Sets _itAddress to the value of the next byte 
                              	//to be written and sets _lastSavedData
  void setVariables(const byte& marker, const MemAddress& itAddress, const SaveData& data);					//For test purposes only, used to set _marker,
  															//_itAddress and _lastSavedData
  void printVariables();
  MemAddress get_Lower_Bound();	//for testing
  MemAddress get_Upper_Bound();
  MemAddress get_Stamp_Size();
  byte get_marker();
  MemAddress get_itAddress();
  SaveData get_lastSavedData();
  bool printRange();
private:
	bool adjust_itAddress();			//called by Read() and Write() to keep 
																//_itAddress within bounds. Returns true if it 
																//was looped, else false.
	const MemAddress _Lower_Bound;//the lowest address of the battery memory. This
																//stores the marker in the marker on the lowest
																//address
	const MemAddress _Upper_Bound;//the highest address of the battery memory
	const int _Stamp_Size;				//how many Bytes in each stamp. This includes
                              	//data and the marker. 4 for each mode, 3 for 
                              	//battery
	const int _Effective_Range;
	byte _marker;									//must be recieved from EEPROM on initialise 
																//and updated everytime i loops back round     
	MemAddress _itAddress;				//iterator is always left at address of the 
																//next address to be written
	SaveData _lastSavedData; 			//stores last saved data, updated every write.
																//set to 0xFFF on power up
};
#endif