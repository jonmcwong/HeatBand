//Allows reading of both hardware Serial and altsoftserial.
//incoming info can be interpretted as either strings or bytes
//Will also write bytes or strings if necessary. 

//must be instantiated with SerialCommsClass<ReplaceWithMyChannel> Comms()

#ifndef Comms1_2_h
#define Comms1_2_h

#ifndef OPERAND_SIZE
#define OPERAND_SIZE 3
#endif

#include "AltSoftSerial.h"
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "EEPROM.h"

template <typename T>//Allows you to give it instantiations of 
																	//the class for different forms of communication

class SerialCommsClass {					//SerialCommsClass<AltSoftSerial> Comms()
public:
	SerialCommsClass (const String& arg, unsigned int timeOut);
																	//Constructor. Unfortunately still needs existing channel instantiation
																	//. "Bytes" or "Strings"
																	//and timeOut


	bool read();										//function will call the relevant methods
																	//read the appropriate buffer as either
																	//Bytes or strings

	bool write(char opcode, const unsigned long operand[OPERAND_SIZE]);		//passes array by reference
																	//format data to be sent and then write to the
																	// output buffer
																	//if bytes used, input parameters must be between 0-255
	
	bool begin(unsigned long baud);	//just like all the other libraries
	bool flush();
	int available();
	char get_opcode();							//return opcode
	const unsigned long* get_operand(int index);	//return element index of operand
	const unsigned long* get_operand();
	String get_command();						//returns the command
	bool get_bytesOrStrings();

private:
	bool _bytesOrStrings;						//0 for bytes, 1 for strings
	bool extractBytes();
	bool extractStrings();
	const unsigned int _Time_Out;		//Time the program waits before it decides that
                          				//it hasn't received the expected termination
	const char _Terminating_Char;		//character marking end of command
																	//is currently a space 
	T* _Channel;			//stores pointer to object

	char _opcode;										
	unsigned long _operand[OPERAND_SIZE];	//template
	String _command;  //received chars stored here
	unsigned int _timeStamp; 

};

#endif