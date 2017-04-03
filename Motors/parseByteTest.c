#include <stdio.h>

int leftLEDCommand = 0;
int rightLEDCommand = 0;

void parseByte(int);
void printLEDCommands();

int main() {

	int receivedByte = 0;

	printLEDCommands();

	parseByte(15);
	printLEDCommands();

	parseByte(31);
	printLEDCommands();




}

void parseByte(int byte) {	
	//binary mask 11110000
	int leftMask = 0b11110000;
	//binary mask 00001111
	int rightMask = 0b00001111;

	//binary AND to get bits for leftMotor
	int tempLeft = (byte & leftMask) >> 4;
	//binary AND to get bits for rightMotor;
	int tempRight = byte & rightMask;

	leftLEDCommand = tempLeft;
	rightLEDCommand = tempRight;
}


void printLEDCommands() {

	printf("leftLED: %d rightLED: %d\n", leftLEDCommand, rightLEDCommand);
}

//parses leftLEDCommand and rightLEDCommand to send forward/reverse to each LED
//two LEDs for each motor, for forward and reverse
void controlLEDs() {

	switch(leftLEDCommand) {
		case 0:

		break;
		case 1:

		break;
		case 2:

		break;
		case 3:

		break;
		case 4:

		break;
		case 5:

		break;
		case 6:

		break;
		case 7:

		break
		case 8:
		break;

		case 9:

		break;
		case 10:

		break;
		case 11:

		break;
		case 12:

		break;

		case 13:

		break;
		case 14:

		break;
		case 15:

		break;
	}
}

