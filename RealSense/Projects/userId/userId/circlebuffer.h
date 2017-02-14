#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "myPoint.h"

#define bufferSize 10

class circleBuffer {
private:
	myPoint locBuffer[bufferSize];
	int writePointer, readPointer; //writePointer points to next location, readPointer points to current location
	double personVector[2]; //2D vector in the x-z plane
public:
	//default constructor
	circleBuffer() { 
		writePointer = 0;
		readPointer = 0;
		locBuffer[bufferSize];
	}
	//method declaration
	void printBuffer();
	void addLocation(myPoint);
	void updatePersonVector();
};

void circleBuffer::printBuffer() {
	printf("-----------Full Buffer--------\n");
	for (int i = 0; i < bufferSize; i++) {
		locBuffer[i].printPoint;
	}
}

void circleBuffer::addLocation(myPoint newLoc) {
	locBuffer[writePointer] = newLoc;
	readPointer = writePointer;
	writePointer = (writePointer + 1) % bufferSize;
	updatePersonVector();
}

void circleBuffer::updatePersonVector(){
	//how to incorporate robot position into this?
}