/*
This version doesn't use the lowest address specifically for the marker
EepromManager.h - Header containing all the relevant functions to do with 
Managing memory usage in the EEPROM when data needs to be written frequently.

-Class basically turns a defined range of memory into one single stamp that can 
be over written again and again.
*/

//Marker is stored at the lowest address in specified range
//All remaining space in range is split into chunks of stamp size.
//Remainder bytes are unused
//For optimal efficiency, _Upper_Bound - _Lower_Bound MUST BE A MULTIPLE OF 
//_Stamp_Size.

//A single stamp looks like this:

//_itAddress
/*----------------------------------------\
|           |             |               |
|   marker  |    Data     |  etc.         |
|           |             |               |
\----------------------------------------*/

//NEED TO WRITE TEST BENCH

#include "EepromManager1_1.h"


EepromManager::EepromManager (MemAddress lowerBound, 	//constructor
															MemAddress upperBound, 
															int stampSize)
: _Lower_Bound (lowerBound),    //intialisation list needed for const
	_Upper_Bound (upperBound),
	_Stamp_Size (stampSize),
	_Effective_Range ((upperBound - lowerBound + 1) / stampSize * stampSize)
	{
		_lastSavedData = 0xFFFF; //to help the read function differentiate between 
															//whether it should read EEPROM or _lastSavedData
															//upon start up
		_itAddress = _Lower_Bound;
	} 

void EepromManager::memoryStatus(const unsigned int& savePeriod) {	//savePeriod is in MilliSeconds
	Serial.flush();
	Serial.print(F("EepromManager uses addresses "));
	Serial.print(_Lower_Bound);
	Serial.print(F(" to "));
	Serial.print(_Upper_Bound);
	Serial.print(F(" with stamp size of "));
	Serial.println(_Stamp_Size);
	Serial.flush();
	Serial.print((_Upper_Bound - _Lower_Bound + 1) % _Stamp_Size);
	Serial.println(F(" bytes are left unused"));
	Serial.flush();
	Serial.print(F("Expected lifetime is: "));
	Serial.print((float)((_Upper_Bound - _Lower_Bound) / _Stamp_Size) / (10800 / savePeriod));
	Serial.println(F(" years"));
	Serial.flush();
	Serial.print("Effective range is: ");
	Serial.println(_Effective_Range);
	//assuming using for 6hrs a day, 50 days a year
	/*
	wasted space is equal to (upper bound - lower bound)%stampsize
	state upperbound and lower bound
	state expected lifetime
	*/
}
	
void EepromManager::write(const SaveData& data) {
	//saves MSByte first
	//>> operator is not destructive
	//byte cast takes least significant 8 bits (hopefully)
	//_itAddress is left on next address to be written
	_lastSavedData = data;
	if (adjust_itAddress()) { //true if corrected
		_marker++;	//increments marker, overflow expected
		Serial.println(F("Incremented marker"));
	}
	EEPROM.update(_itAddress++, _marker); 								//write marker at beginning of each stamp
	for (int i = _Stamp_Size - 1; i > 0; i--) { 				//i is remaining bytes to write
		EEPROM.update(_itAddress++, (byte)(data >> (i - 1) * 8)); //write byte
	}
}

EepromManager::SaveData EepromManager::read() {
	//Almost exactly the inverse of write
	//_itAddress is left on next address to be written
	if (_lastSavedData != 0xFFFF) { 										//if not on power up
	return _lastSavedData;
	} 
	SaveData reconstructedData = 0;
	_itAddress = _itAddress - _Stamp_Size; 			//set to first byte to read
	adjust_itAddress();
	_itAddress++;
	for (int i = _Stamp_Size - 1; i > 0; i--) { 				//i is remaining bytes to read
		reconstructedData |= ((SaveData)EEPROM.read(_itAddress++)) << ((i - 1) * 8);
		//concatenate bytes with appropriate byte shifts
	}
	adjust_itAddress(); //should be back to where we started
	_lastSavedData = reconstructedData;
	return reconstructedData;
}

void EepromManager::begin() {
	_marker = EEPROM.read(_Lower_Bound);
	_itAddress = _Lower_Bound;
	while (EEPROM.read(_itAddress) == _marker && _itAddress < _Upper_Bound) {
		_itAddress += _Stamp_Size;
	} 
	adjust_itAddress();// incase _itAddress is out of bounds
	//when it exits the loop, _itAddress will be left at the address of the 
	//next byte to be written
	read();
}

bool EepromManager::adjust_itAddress() {				//called by Read() and Write(). Keeps
																//_itAddress within bounds. Returns true if it 
																//was looped, else false.
	//whole range - wasted bytes
	//or highest mutliple of stamp size
	if (_itAddress >= _Lower_Bound && _itAddress < _Lower_Bound + _Effective_Range) {
		//if in bounds do nothing
		return false;
	}
	_itAddress = (((_itAddress - _Lower_Bound) % _Effective_Range + _Effective_Range) % _Effective_Range) /_Stamp_Size * _Stamp_Size + _Lower_Bound;
	//long expression is needed because % operator in C++ is not true modulo
	return true;
	
	//basically loops it
	//first remove offset from _Lower_Bound and marker
	//mod the result so that it fits within effectiveRange
	//add offset back
	
}

void EepromManager::setVariables(const byte& marker, const MemAddress& itAddress, const SaveData& data) { //For test purposes only, used to set _marker,
	_marker = marker;
	_itAddress = itAddress;
	_lastSavedData = data;
}					
  															//_itAddress and _lastSavedData
EepromManager::MemAddress EepromManager::get_Lower_Bound() {
	return _Lower_Bound;
}
EepromManager::MemAddress EepromManager::get_Upper_Bound() {
	return _Upper_Bound;
}
EepromManager::MemAddress EepromManager::get_Stamp_Size() {
	return _Stamp_Size;
}
byte EepromManager::get_marker() {
	return _marker;
}
EepromManager::MemAddress EepromManager::get_itAddress() {
	return _itAddress;
}
EepromManager::SaveData EepromManager::get_lastSavedData() {
	return _lastSavedData;
}

bool EepromManager::printRange () {
	Serial.begin(9600);
	Serial.println("Print range:");
  Serial.println("Address\t\t\t\tStored Value");
  for (int _itAddress = _Lower_Bound; _itAddress <= _Upper_Bound; _itAddress++) {
	  Serial.print(_itAddress);
	  Serial.print("\t\t\t\t");			
	  Serial.println(EEPROM.read(_itAddress));
	}
  return true;
}

void EepromManager::printVariables () {
	Serial.print("Marker: ");
	Serial.println(_marker);
	Serial.print("itAddress: ");
	Serial.println(_itAddress);
	Serial.print("lastSavedData: ");
	Serial.println(_lastSavedData);
}    