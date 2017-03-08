#pragma once
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "pxcpersontrackingdata.h"

#define myBufferSize 3


//https://software.intel.com/sites/landingpage/realsense/camera-sdk/v1.1/documentation/html/index.html?pointcombined_persontracking_pxcpersontrackingdata.html
//PXCPersonTrackingData::PersonTracking::PointCombined
/*struct location { //replace this with PXCPersonTrackingData::PersonTracking::PointCombined
	double x;
	double y;
	double distance;
};*/

PXCPersonTrackingData::PersonTracking::PointCombined buffer[myBufferSize];


class myBuffer {

private:
	int curr;
	bool isInitialized;
public:
	//primary constructor
	myBuffer() {
		curr = 0;
		isInitialized = false;
	}
	//secondary constructor
	myBuffer(PXCPersonTrackingData::PersonTracking::PointCombined newLoc) {
		curr = 0;
		isInitialized = false;
		add(newLoc);
	}
	void add(PXCPersonTrackingData::PersonTracking::PointCombined);
	PXCPersonTrackingData::PersonTracking::PointCombined getLastLocation();
	void printBuffer();
	//void printLocation();

};


PXCPersonTrackingData::PersonTracking::PointCombined createLocation(double x, double y, double distance) { //creates a new location struct
	PXCPersonTrackingData::PersonTracking::PointCombined newLocation;
	newLocation.world.point.x = x;
	newLocation.world.point.y = y;
	newLocation.world.point.z = distance;
	return newLocation;
}

void myBuffer::add(PXCPersonTrackingData::PersonTracking::PointCombined newLocation) {
	if (!isInitialized) {
		isInitialized = true;
	}

	switch (curr) {
	case 0:
		buffer[0] = newLocation;
		curr = 1;
		break;

	case 1:
		buffer[1] = newLocation;
		curr = 2;
		break;

	case 2:
		buffer[2] = newLocation;
		curr = 0;
		break;
	}

}

PXCPersonTrackingData::PersonTracking::PointCombined myBuffer::getLastLocation() { //return target user's last known location, NULL if no location data entered
									   //will return garbage data if called before add
	//printf("lastLocation:  \n");
	//printf("curr=%d\n", curr);

	PXCPersonTrackingData::PersonTracking::PointCombined toReturn;
	if (!isInitialized) {
		return createLocation(-1, -1, -1);
	}
	switch (curr) {
	case 0:
		toReturn = buffer[2];
		break;

	case 1:
		toReturn = buffer[0];
		break;

	case 2:
		toReturn = buffer[1];
		break;
	}
	return toReturn;
}

void printLocation(PXCPersonTrackingData::PersonTracking::PointCombined toPrint) {
	printf("x: %.2f y: %.2f z: %.2f\n", toPrint.image.point.x, toPrint.image.point.y, toPrint.world.point.z * 1000); //1000 multiplier to normalize z distance with joint units
}

void myBuffer::printBuffer() {
	printf("curr=%d\n", curr);
	printf("   buffer[%d]=   ", 0); printLocation(buffer[0]);
	printf("   buffer[%d]=   ", 1); printLocation(buffer[1]);
	printf("   buffer[%d]=   ", 2); printLocation(buffer[2]);

}
