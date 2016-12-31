#include "myPoint.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

myPoint::myPoint(): x(0), y(0), z(0) {

}

void myPoint::updateValues(double newX, double newY, double newZ) {
	x = newX;
	y = newY;
	z = newZ;
}

void myPoint::printPoint() {
	printf("     (%.2f, %.2f, %.2f)\n", x, y, z);
}

/*int main() {
	printf("Hello World\n");
	myPoint testPoint;
	testPoint.x = 7;
	testPoint.printPoint();
	testPoint.updateValues(1,2,3);
	testPoint.printPoint();



	//myPoint::printPoint(testPoint);

}*/