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

/* Global variables for motor control 
	Current agreed upon motor configuration:

	0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15 
	 <------Reverse     | |   Forward-----> 		 |	

int leftMotor = 0;
int rightMotor = 0;


/* Function declarations */
void packageMotorControlByte();
void sendControls();
void goForward();
void turnLeft();
void turnRight();


/* Converts leftMotor control variables into one bit to send to Arduino */
void packageMotorControlByte() {
	toWrite = (char*)(rightMotor + leftMotor << 4);
}

/* Packages and sends the motor controls */



void goForward() {  //medium speed forward
	leftMotor = 10;
	rightMotor = 10; 
}

void goBackwards() {
	leftMotor = 3;
	rightMotor = 3;
}

void turnLeft() {  //turns left
	//left motor reverse
	leftMotor = 3
	//right motor forward
	rightMotor = 10;
}

void turnRight() {
	//left motor forward
	leftMotor = 10;
	//right motor reverse
	rightMotor = 3;
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

		packa

		// else {
		// 	printf("%u\n", *toWrite);
		// }

		Sleep(500);
		//(*toWrite)++; //dereferences and increments value at the pointer

	}
}