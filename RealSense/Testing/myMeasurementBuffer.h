#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#define bufferSize 3


//https://software.intel.com/sites/landingpage/realsense/camera-sdk/v1.1/documentation/html/index.html?pointcombined_persontracking_pxcpersontrackingdata.html
struct measurement { //replace this with PXCPersonTrackingData::PersonTracking::PointCombined
	double shoulderWidth;
	double torsoHeight;
};

class myMeasurementBuffer {

	private:
		int curr;
		bool isInitialized;
	public:
	//primary constructor
	myMeasurementBuffer() {
		curr = 0;
		isInitialized = false;
	}
	//secondary constructor
	myMeasurementBuffer(measurement newMeasurement) {
		curr = 0;
		isInitialized = false;
		add(newLocation);
	}
	void add(location);
	location getLastLocation();
	void printBuffer();
	//void printLocation();

};

location buffer[bufferSize];

location createLocation(double x, double y) { //creates a new location struct
	location newLocation;
	newLocation.x = x;
	newLocation.y = y;
	return newLocation;
}

void myMeasurementBuffer::add(location newLocation) {
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

location myMeasurementBuffer::getLastLocation() { //return target user's last known location, NULL if no location data entered
	//will return garbage data if called before add
	printf("lastLocation:  \n");
	printf("curr=%d\n", curr);
	
	location toReturn;
	if (!isInitialized) {
		return createLocation(-1, -1);
	}
	switch (curr) {
		case 0:
		printf("buf[2]\n");
		toReturn = buffer[2];
		break;
		
		case 1:
		printf("buf[0]\n");		
		toReturn = buffer[0];
		break;

		case 2:
		printf("buf[1]\n");
		toReturn = buffer[1];
		break;
	}
	return toReturn;
}

void printLocation(location toPrint) {
	printf("shoulderWidth: %.2f torsoHeight: %.2f\n", toPrint.shoulderWidth, toPrint.torsoHeight);
}

void myMeasurementBuffer::printBuffer() {
	printf("curr=%d\n", curr);
	printf("   buffer[%d]=   ", 0); printLocation(buffer[0]);
	printf("   buffer[%d]=   ", 1); printLocation(buffer[1]);
	printf("   buffer[%d]=   ", 2); printLocation(buffer[2]);

}
