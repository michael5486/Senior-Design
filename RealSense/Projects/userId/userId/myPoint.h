#include <stdio.h>
#include <iostream>
#include <stdlib.h>

class myPoint {
	private:
		double worldX, worldY, worldZ, imageX, imageY;
	public:
		//Default Constructor
		myPoint(double a = 0, double b = 0, double c = 0, double d = 0, double e = 0) {
			worldX = a;
			worldY = b;
			worldZ = c; 
			imageX = d;
			imageY = e;
		}
		void printPoint();
		void updateValues(double , double , double, double, double);
		double getWorldX() { return worldX; }
		double getWorldY() { return worldY; }
		double getWorldZ() { return worldZ; }
		double getImageX() { return imageX; }
		double getImageY() { return imageY; }
};

void myPoint::updateValues(double newWorldX, double newWorldY, double newWorldZ, double newImageX, double newImageY) {
	worldX = newWorldX;
	worldY = newWorldY;
	worldZ = newWorldZ;
	imageX = newImageX;
	imageY = newImageY;
}

void myPoint::printPoint() {
	printf("   World: (%.2f, %.2f, %.2f)\n", worldX, worldY, worldZ);
	printf("                               Image: (%.2f, %.2f)\n", imageX, imageY);

}