#include <stdio.h>
#include <iostream>
#include <stdlib.h>

class myPoint {
	private:
		double x, y, z;
	public:
		//Default Constructor
		myPoint(double a = 0, double b = 0, double c = 0) {
			x = a;
			y = b;
			z = c; 
		}
		void printPoint();
		void updateValues(double , double , double);
		double getX() { return x; }
		double getY() { return y; }
		double getZ() { return z; }
};

void myPoint::updateValues(double newX, double newY, double newZ) {
	x = newX;
	y = newY;
	z = newZ;
}

void myPoint::printPoint() {
	printf("     (%.2f, %.2f, %.2f)\n", x, y, z);
}