#include "myPoint.h"
#include <math.h>

int personCounter = 0; //global variable, increments for each new person constructed

class myPerson{
	private:
		int personID;
		double shoulderDistance, height, leftArmLength, rightArmLength;
		myPoint JOINT_HEAD;
		myPoint JOINT_SHOULDER_LEFT;
		myPoint JOINT_SHOULDER_RIGHT;
		myPoint JOINT_HAND_LEFT;
		myPoint JOINT_HAND_RIGHT;
		myPoint JOINT_HIP_LEFT;
		myPoint JOINT_HIP_RIGHT;
		myPoint JOINT_FOOT_LEFT;
		myPoint JOINT_FOOT_RIGHT;

	public: 
		myPerson() : JOINT_HEAD(), JOINT_SHOULDER_LEFT(), JOINT_SHOULDER_RIGHT(), JOINT_HAND_LEFT(),
			JOINT_HAND_RIGHT(), JOINT_HIP_LEFT(), JOINT_HIP_RIGHT(), JOINT_FOOT_LEFT(), JOINT_FOOT_RIGHT() {
			personID = personCounter++;
			shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);
		}
		void printPerson();
		void updateJoints(myPoint, myPoint, myPoint, myPoint, myPoint);
		double calculateDistance(myPoint, myPoint);
		myPoint calculateMidpoint(myPoint, myPoint);
		myPoint getLeftFoot() { return JOINT_FOOT_LEFT; }
		myPoint getRightFoot() { return JOINT_FOOT_RIGHT; }

};

void myPerson::updateJoints(myPoint newHead, myPoint newShoulderLeft, myPoint newShoulderRight, myPoint newFootLeft, myPoint newFootRight) {
	JOINT_HEAD = newHead;
	JOINT_SHOULDER_LEFT = newShoulderLeft;
	JOINT_SHOULDER_RIGHT = newShoulderRight;
	JOINT_FOOT_LEFT = newFootLeft;
	JOINT_FOOT_RIGHT = newFootRight;

	shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);
	
	myPoint midFeet = calculateMidpoint(JOINT_FOOT_LEFT, JOINT_FOOT_RIGHT);
	height = calculateDistance(JOINT_HEAD, midFeet);

	leftArmLength = calculateDistance(J)

}

void myPerson::printPerson() {
	printf("Printing person%d:\n", personID);
	printf("  Joint Locations:\n");
	printf("    JOINT_HEAD:           "); JOINT_HEAD.printPoint();
	printf("    JOINT_SHOULDER_LEFT:  "); JOINT_SHOULDER_LEFT.printPoint();
	printf("    JOINT_SHOULDER_RIGHT: "); JOINT_SHOULDER_RIGHT.printPoint();
	printf("    JOINT_FOOT_LEFT:      "); JOINT_FOOT_LEFT.printPoint();
	printf("    JOINT_FOOT_RIGHT:     "); JOINT_FOOT_RIGHT.printPoint();
	printf("  Calculated Features:\n");
	printf("    shoulderDistance:     %.2f\n", shoulderDistance);
	printf("    height:               %.2f\n", height);


}

double myPerson::calculateDistance(myPoint point1, myPoint point2) { //order of params doesn't matter
	double difX = point1.getX() - point2.getX();
	double difY = point1.getY() - point2.getY();
	double difZ = point1.getZ() - point2.getZ();
	double sumXYZ = pow(difX, 2) + pow(difY, 2) + pow(difZ, 2);
	return sqrt(sumXYZ);
}

myPoint myPerson::calculateMidpoint(myPoint point1, myPoint point2) { //order of params doesn't matter
	double difX = fabs(point1.getX() - point2.getX()); //float absolute value function
	double difY = fabs(point1.getY() - point2.getY());
	double difZ = fabs(point1.getZ() - point2.getZ());
	double finalX, finalY, finalZ;
	if (point1.getX() < point2.getX()) { //use smaller x value, add to it
		finalX = point1.getX() + (difX / 2);
	}
	else {
		finalX = point2.getX() + (difX / 2);
	}
	if (point1.getY() < point2.getY()) { //use smaller y value, add to it
		finalY = point1.getY() + (difY / 2);
	}
	else {
		finalY = point2.getY() + (difY / 2);
	}
	if (point1.getZ() < point2.getZ()) { //use smaller z value, add to it
		finalZ = point1.getZ() + (difZ / 2);
	}
	else {
		finalZ = point2.getZ() + (difZ / 2);
	}
	myPoint toReturn(finalX, finalY, finalZ);
	return toReturn;
}