//Allows reading of both hardware and altsoftserial.
//incoming info can be interpretted as either strings or bytes

//changes to be made
//take out the template arg in constructor
//create explicit secialization 
//make operand size adjustable
  //make sure write takes different data
//takes only 3 operands, terminating char is ' '
#include "Comms1_2.h"
#include "AltSoftSerial.h"
template <typename T>
SerialCommsClass<T>::SerialCommsClass (const String& arg, unsigned int timeOut)   //constructor

: _Time_Out (timeOut),    //upto 32 seconds
	_Terminating_Char (' ')
	{
    static T channel;
    _Channel = &channel;
    if (arg == "Bytes") {
      _bytesOrStrings = 0;    //try to make sure that realloc() is not called. Try to avoid fragmenting the heap
      _command.reserve(OPERAND_SIZE + 2);
    } else if (arg == "Strings") {
      _bytesOrStrings = 1;      //try to make sure that realloc() is not called. Try to avoid fragmenting the heap
      _command.reserve(OPERAND_SIZE * 5 + 2);
    } else {
      Serial.println(F("Invalid arg"));
    }
  }

template <>
SerialCommsClass<HardwareSerial>::SerialCommsClass (const String& arg, unsigned int timeOut)   //explicit specialization of constructor
: _Time_Out (timeOut),
  _Terminating_Char (' ')
  {
    _Channel = &Serial;
    if (arg == "Bytes") {
      _bytesOrStrings = 0;    //try to make sure that realloc() is not called. Try to avoid fragmenting the heap
      _command.reserve(OPERAND_SIZE + 2);
    } else if (arg == "Strings") {
      _bytesOrStrings = 1;      //try to make sure that realloc() is not called. Try to avoid fragmenting the heap
      _command.reserve(OPERAND_SIZE * 5 + 2);
    } else {
      Serial.println(F("Invalid arg"));
      Serial.flush();
    }
  }

template <typename T>
bool SerialCommsClass<T>::read () {  //Takes either Serial or an instantiation of AltSerial 
                                                              //arg is either "Bytes" or "Strings"
  if (_command.length() == 0) {     //time out tested
    //Serial.flush();
    _timeStamp = (unsigned int)millis();    //equivalent to mod 65536
    //resets every call
  } else if ((unsigned int)((unsigned int)millis() - _timeStamp) > _Time_Out) {
    //if it doesn't receive the terminating char after expected time
    Serial.println(F("Received incomplete command")); //No termination
    Serial.println(_command);
    _command = ""; //gets rid of data
    //perhaps should ask to resend command
  } 

  while (_Channel->template available()) {
    char c = _Channel->template read();
    _command += c;      //add to command
    if (c == ' ') {
      break;
    }
    Serial.print(c);
  }
return _bytesOrStrings ? extractStrings() : extractBytes(); //return appropriate interpretation
}

template <typename T>
bool SerialCommsClass<T>::extractBytes () { 
  //takes the command and turns into an opcode and operand
  if (_command.length() == 0) {
    //if the string is empty or is not terminated, 
    //a complete command hasn't been received yet
    return false;
  } else if (_command[_command.length() - 1] != _Terminating_Char) {
    //not terminated
    return false;
  } else if (_command.length() != OPERAND_SIZE + 2) {    //2 bytes for opcode and delimiter
    //if something weird is received
    Serial.println(F("Invalid data length"));
    _command = "";
    return false;
  }
  //must be a valid command
  _opcode = _command[0]; //expects the first character to be an opcode
  for (unsigned int i = 0; i < OPERAND_SIZE; i++) {
    _operand[i] = _command[i + 1];
  }
  _command = ""; //Finished using command, so delete it. All info is in operand and opcode
  return true;
}

template <typename T>
bool SerialCommsClass<T>::write (char opcode, const unsigned long operand[OPERAND_SIZE]) {
  //Specifying array size makes no difference
  static String buffer;       //reserves space
  buffer = "";      //empties buffer
  if(_bytesOrStrings) {
//allows for delimiters and average 4 chars per number
    buffer.reserve(OPERAND_SIZE * 5 + 3);
  } else {
//accounts for 1 byte per input, delimiters and char
    buffer.reserve(OPERAND_SIZE + 3);
  }
  buffer += '?';
  buffer += opcode;
  if (_bytesOrStrings) {        //for strings
    for (unsigned int i = 0; i < OPERAND_SIZE; i++) {
      buffer += String(operand[i]);        //constructor used to convert between ulong and string
      buffer += ",";
    }
    buffer[buffer.length() - 1] = '!';        //gets rid of extra comma at the end
  } else { //must be bytes
    if (operand[0] > 255 || operand[1] > 255 || operand[2] > 255) {
      return false;
    }
    for (unsigned int i = 0; i < OPERAND_SIZE; i++) {
      buffer += (char)operand[i];
    }
    buffer += '!';
  }
  for (unsigned int i = 0; i < buffer.length(); i++) {
    _Channel->template write(buffer[i]);       //read byte by byte necessary fo compatibility between the 2 different 
                                                //implementations of altsoftserial and HardwareSerial
  }
  return true;
}

template <typename T>
bool SerialCommsClass<T>::begin (unsigned long baud) {
  _Channel->template begin(baud);
  return true;
}

template <typename T>
bool SerialCommsClass<T>::flush() {
  _Channel->template flush();
  return true;
}

template <typename T>
int SerialCommsClass<T>::available() {
  return _Channel->template available();
}

template <typename T>
bool SerialCommsClass<T>::extractStrings () {
	  //takes the command and turns into an opcode and operand
  if (_command.length() == 0 || _command[_command.length() - 1] != _Terminating_Char) {
    //if the string is empty or is not terminated, 
    //a complete command hasn't been received yet
    //never have segmentation fault due to how the || operator is defined
    return false;
  }

  //validate number of commas
  //if good, break into sub strings with for loop saving the value of the last comma
  int noOfCommas = -1;
  for (int indexOfLastComma = 0; indexOfLastComma != -1; noOfCommas++) {
    indexOfLastComma = _command.indexOf(',',indexOfLastComma + 1);
  }       //returns number of commas 
  if (noOfCommas + 1 != OPERAND_SIZE) {
    Serial.println(F("Not enough operands"));
    Serial.println(_command);
    _command = "";    //throws data away
    return false;
  }
  int indexOfLastComma = 0;
  for (int i = 0; i < OPERAND_SIZE - 1; i++) {
    int indexOfNextComma = _command.indexOf(',', indexOfLastComma + 1);
    _operand[i] = _command.substring(indexOfLastComma + 1, indexOfNextComma).toInt();
    indexOfLastComma = indexOfNextComma;
  }   //takes the string from 1 after the last comma to the next comma and converts to int. repeats.
  _operand[OPERAND_SIZE - 1] = _command.substring(indexOfLastComma + 1, _command.length() - 1).toInt();
  //Completes last piece of data. Stops before terminating char
  _opcode = _command[0];
  _command = "";
  return true;            //needs minimum of char ',' ',' ' '.
}

template <typename T>
char SerialCommsClass<T>::get_opcode () {
	return _opcode;        //pass by value
}

template <typename T>
const unsigned long* SerialCommsClass<T>::get_operand (int index) {
	return &_operand[index];       //pass pointer
}

template <typename T>
const unsigned long* SerialCommsClass<T>::get_operand () {
  return get_operand(0);        //should return a pointer to the first element in the array
}

template <typename T>
String SerialCommsClass<T>::get_command () {
	return _command;
}

template <typename T>
bool SerialCommsClass<T>::get_bytesOrStrings() {
  return _bytesOrStrings;
}
//instantiated here
template class SerialCommsClass<AltSoftSerial>;
template class SerialCommsClass<HardwareSerial>;