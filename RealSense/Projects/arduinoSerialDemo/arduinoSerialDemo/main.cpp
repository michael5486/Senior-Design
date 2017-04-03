#include <stdio.h>
#include <string>
#include <Windows.h>
#include "SerialClass.h"	// Serial C++ arduino library


/* Application writes bytes if information to the Serial port. Bytes are read and parsed by
   an Arduino and converted into motor movements to followthe target user */

#define ARDUINO_PORT "COM4"

   /* Global variable declarations for serial communications to Arduino */
Serial* SP = new Serial(ARDUINO_PORT);
char *toWrite;

/* Global variables for motor control */
int leftMotor = 0;
int rightMotor = 0;

/* Function declarations */
void packageMotorControlByte();
void goForward();
void turnLeft();
void turnRight();


/* Converts leftMotor control variables into one bit to send to Arduino */
void packageMotorControlByte() {
	toWrite = (char*)(rightMotor + leftMotor << 4);
}

void goForward() {


}

void turnLeft() {


}

void turnRight() {


}




int main() {

	toWrite = new char[1]; //allocates memory for out character buffer
	*toWrite = 0; //sets it equal to zero

	if (SP->IsConnected())
		printf("Connected to %s\n", ARDUINO_PORT);
	else {
		printf("Cannot find Arduino, please try again.");
	}


	boolean success = true; //tracks if info successfully sent
	int nbChar = 1; //all messages sent is 1 char long
	

	while (SP->IsConnected()) { //while the Arduino is connected

		if (SP->WriteData(toWrite, nbChar) == false) { //information unsuccessfully transmitted
			printf("Error in Arduino communication\n");
		}
		else {
			printf("%u\n", *toWrite);
		}
		Sleep(500);
		(*toWrite)++; //dereferences and increments value at the pointer

	}
}