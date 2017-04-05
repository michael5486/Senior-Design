#include <stdio.h>
#include <string>
#include <Windows.h>
#include <conio.h> //for _getch
//#include <bitset> //for converting ints to bits
#include <iostream>
#include "SerialClass.h"	// Serial C++ arduino library

using namespace std;


/* Application writes bytes if information to the Serial port. Bytes are read and parsed by
   an Arduino and converted into motor movements to followthe target user */


/* Make sure to check the Arduino port before usage */
#define ARDUINO_PORT "COM3"

   /* Global variable declarations for serial communications to Arduino */
Serial* SP = new Serial(ARDUINO_PORT);
char *toWrite;
boolean success = true; //tracks if info successfully sent
int nbChar = 1; //all messages sent is 1 char long

/* Global variables for motor control 
	Current agreed upon motor configuration:

	0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15 
	 <------Reverse     | |   Forward-----> 		 |	*/

unsigned int leftMotor = 0;
unsigned int rightMotor = 0;

/* Function declarations */
void packageMotorControlByte();
void sendControls();
void goForward();
void turnLeft();
void turnRight();


/* Converts leftMotor and rightMotor control variables into one byte to send to Arduino */
/* When you print out as int, they become signed and obtain negative values...doesn't matter still same binary representation*/
void packageMotorControlByte() {
	unsigned int temp = leftMotor << 4;
	printf("leftMotor = %d  rightMotor = %d\n", leftMotor, rightMotor);
	//printf("leftMotor << 4 = %d\n", temp);
//	printf("rightMotor + (leftMotor << 4)= %d\n", rightMotor + temp);
//	printf("rightMotor + (leftMotor << 4)= %c\n", rightMotor + temp);
	*toWrite = rightMotor + temp;
	//printf("toWrite = %d\n", *toWrite);
}

/* Packages and sends the motor controls */
void sendControls() {
	packageMotorControlByte();
	if (SP->WriteData(toWrite, nbChar) == false) { //information unsuccessfully transmitted
		printf("Error in Arduino communication\n");
	}
}

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
	leftMotor = 3;
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
		printf("Cannot find Arduino, please try again.\n");
	}

	while (SP->IsConnected()) { //while the Arduino is connected
	//while(true) {
		if (_kbhit()) { // Break loop
			int c = _getch() & 255;
			switch (c) {

				case 72: //up arrow, go forward
					printf("go forward...\n");
					goForward();
					sendControls();
				break;
				case 77: //right arrow, turn right
					printf("go right...\n");
					turnRight();
					sendControls();
				break;
				case 80: //down arrow, go backwards
					printf("go reverse...\n");
					goBackwards();
					sendControls();
				break;
				case 75: //left arrow, go left
					printf("go left...\n");
					turnLeft();
					sendControls();
				break;
				case 'q': //q to quit
					return 0;
				break;
			}
		}
	}
}