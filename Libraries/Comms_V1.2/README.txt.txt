Comms.h allows reading of both hardware and altsoftserial serial ports.
(Should technically be able to read SoftwareSerial too)
Incoming info can be interpretted as either strings or bytes

Instantiation of class must be made to be used. 


It's callable methods are:

	SerialCommsClass (arg, timeOut)
		-Constructs an object of SerialCommsClass

	read(channel, arg)
		-If there is stuff in the buffer read it all.
		-Once a full command is received, extract all the information from it
		Parameters
			channel
				-Should either be Serial or name of AltSoftSerial instantiation
			arg
				-"Strings" or "Bytes"
			Return Value
				-True if valid command is received, false otherwise.

	get_opcode()
		Returns last opcode received.
		Parameters
			-none
		Return Value
			-opcode

	get_operand(index)
		Returns an element in operand 
		Parameters
			index
				-refers to the index of operand[] that is to be returned
		Return Values
			-value of operand at index