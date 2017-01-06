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
		myPoint JOINT_SPINE_MID;

	public: 
		myPerson() : JOINT_HEAD(), JOINT_SHOULDER_LEFT(), JOINT_SHOULDER_RIGHT(), JOINT_HAND_LEFT(),
			JOINT_HAND_RIGHT(), JOINT_SPINE_MID() {
			personID = personCounter++;
			//shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);

		}
		void printPerson();
		void updateJoints(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
		double calculateDistance(myPoint, myPoint);
		//myPoint calculateMidpoint(myPoint, myPoint);
		// Center of area calc already part of person tracking module

};

void myPerson::updateJoints(myPoint newHead, myPoint newSpineMid, myPoint newShoulderLeft, myPoint newShoulderRight, myPoint newHandLeft, myPoint newHandRight) {
	JOINT_HEAD = newHead;
	JOINT_SPINE_MID = newSpineMid;
	JOINT_SHOULDER_LEFT = newShoulderLeft;
	JOINT_SHOULDER_RIGHT = newShoulderRight;
	JOINT_HAND_LEFT = newHandLeft;
	JOINT_HAND_LEFT = newHandRight;

	//shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);
	armLength = mean(calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT), calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT));
	height = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID);

}

void myPerson::printPerson() {
	printf("Printing person%d:\n", personID);
	printf("  Joint Locations:\n");
	printf("    JOINT_HEAD:           "); JOINT_HEAD.printPoint();
	printf("    JOINT_SPINE_MID:      "); JOINT_SPINE_MID.printPoint();
	printf("    JOINT_SHOULDER_LEFT:  "); JOINT_SHOULDER_LEFT.printPoint();
	printf("    JOINT_SHOULDER_RIGHT: "); JOINT_SHOULDER_RIGHT.printPoint();
	printf("    JOINT_HAND_LEFT:      "); JOINT_HAND_LEFT.printPoint();
	printf("    JOINT_HAND_RIGHT:     "); JOINT_HAND_RIGHT.printPoint();
	printf("  Calculated Features:\n");
	printf("    ArmLength:            %.2f\n", shoulderDistance);
	printf("    UpperBodyHeight:      %.2f\n", height);


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