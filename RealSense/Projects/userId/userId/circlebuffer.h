#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "myPoint.h"
#include <math.h>

#define bufferSize 10

int writePointer = 0;
int readPointer = 0;


location locBuffer[bufferSize]; //creates the buffer


location createLocation(double x, double y, double distance) { //creates a new location struct
	location newLocation;
	newLocation.x = x;
	newLocation.y = y;
	newLocation.distance = distance;
	return newLocation;
}

void createBuffer() { //initializes the buffer
	for (int i = 0; i < bufferSize; i++) {
		location tempLocation = createLocation(0, 0, 0);
		locBuffer[i] = tempLocation;
	}

}

void printLocation(location toPrint) {
	printf("x: %.2f y: %.2f distance: %.2f\n", toPrint.x, toPrint.y, toPrint.distance);
}

void printBuffer() {
	printf("-----------Full Buffer--------\n");
	for (int i = 0; i < bufferSize; i++) {
		printLocation(locBuffer[i]);
	}

}

void increaseReadPtr() {
	readPointer = (readPointer + 1) % bufferSize; //increases readPointer and wraps around to beginning
}

void increaseWritePtr() {
	writePointer = (writePointer + 1) % bufferSize; //increases writePointer and wraps around to beginning
	readPointer - writePointer; //we want readPointer to stay consistent with readpointer
}

void decreaseReadPtr() {
	readPointer = readPointer - 1;
	if (readPointer == -1) {
		readPointer = bufferSize;
	}
}

/*may want to have a function here that looks at the last couple of locations and
use them to decide which side of the screen the user left from*/

void main() {

	/* initialize random seed: */
	srand(time(NULL));

	int pause;

	location newLocation = createLocation(50, 50, 2);
	printLocation(newLocation);

	createBuffer();
	printBuffer();


	while (true) {
		location newLocation = createLocation(rand() % 100, rand() % 100, rand() % 100);
		locBuffer[writePointer] = newLocation;
		increaseWritePtr();
		Sleep(100);
		printBuffer();

	}


	std::cout << "Press any key to end...\n";
	std::cin >> pause;

}