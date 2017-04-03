#include <stdio.h>
#include <iostream>
#include <stdlib.h>
//#include "myPoint.h"

#define bufferSize 3

class circleBuffer {
private:
	myPoint locBuffer[bufferSize];
	int writePointer, currPos, lastPos; //writePointer points to next location, currPos points to current location, lastPos points to previous location
	double ULV[2]; //2D user location vector in the x-z plane
public:
	//default constructor
	circleBuffer() {
		writePointer, currPos, lastPos = 0;
		locBuffer[bufferSize];
	}
	//method declaration
	void printBuffer(); //print buffer
	void startBuffer(myPoint newLoc); //begins buffer after TU initialization
	void updateULV(myPoint newLoc, double[]); //update user location vector, input is vector of robot motion <rx,rz>
	myPoint returnLKL() { return locBuffer[currPos]; }//return last known location
	double returnULV() { return ULV[2]; } //returns User Location Vector
};

void circleBuffer::printBuffer() {
	printf("-----------Full Buffer--------\n");
	for (int i = 0; i < bufferSize; i++) {
		locBuffer[i].printPoint();
	}
}

void circleBuffer::startBuffer(myPoint newLoc) { //begin buffer
	locBuffer[writePointer] = newLoc;
	lastPos = currPos;
	currPos = writePointer;
	writePointer = (writePointer + 1) % bufferSize;
}

void circleBuffer::updateULV(myPoint newLoc, double robVector[]) { //add locations to buffer
	locBuffer[writePointer] = newLoc;
	lastPos = currPos;
	currPos = writePointer;
	writePointer = (writePointer + 1) % bufferSize;
	/*robVector contains rx and rz, which describe movement of robot from lastPos to currPos
	currPos and lastPos are relative to position of robot, we want currPos relative to the robot's
	position when user was at lastPos*/
	myPoint last = locBuffer[lastPos];
	myPoint curr = locBuffer[currPos];
	double currx = curr.getWorldX(), currz = curr.getWorldZ()*1000;
	currx += robVector[0];
	currz += robVector[1];
	ULV[0] = currx - last.getWorldX();
	ULV[1] = currz - last.getWorldZ()*1000;
}