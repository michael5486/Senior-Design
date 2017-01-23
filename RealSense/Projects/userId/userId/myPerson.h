#include "myPoint.h"
#include <math.h>

int personCounter = 0; //global variable, increments for each new person constructed

class myPerson{
	public: 
		//int personID; //unneccessary using my method
		//making all of the members public allows easier access, I don't wanna make a bunch more functions just to access these
		double shoulderDistance, leftArmLength, rightArmLength, torsoHeight;
		myPoint JOINT_HEAD;
		myPoint JOINT_SHOULDER_LEFT;
		myPoint JOINT_SHOULDER_RIGHT;
		myPoint JOINT_HAND_LEFT;
		myPoint JOINT_HAND_RIGHT;
		myPoint JOINT_SPINE_MID;
		//Primary Constructor
		myPerson(myPoint head, myPoint lShoulder, myPoint rShoulder, 
				myPoint lHand, myPoint rHand, myPoint midSpine) {
			JOINT_HEAD = head;
			JOINT_SHOULDER_LEFT = lShoulder;
			JOINT_SHOULDER_RIGHT = rShoulder;
			JOINT_HAND_LEFT = lHand;
			JOINT_HAND_RIGHT = rHand;
			JOINT_SPINE_MID = midSpine;
			//personID = personCounter++;
			shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);
			leftArmLength = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT);
			rightArmLength = calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT);
			torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID);
		}
		//Default Constructor, sets all members to 0
		myPerson() {
			myPoint head, lShoulder, rShoulder, lHand, rHand, midSpine;
			JOINT_HEAD = head;
			JOINT_SHOULDER_LEFT = lShoulder;
			JOINT_SHOULDER_RIGHT = rShoulder;
			JOINT_HAND_LEFT = lHand;
			JOINT_HAND_RIGHT = rShoulder;
			JOINT_SPINE_MID = midSpine;
			
			//personID = personCounter++;
			shoulderDistance = 0;
			leftArmLength = 0;
			rightArmLength = 0;
			torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID);


		}
		void printPerson();
		void updateJoints(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
		double calculateDistance(myPoint, myPoint);
		myPoint calculateMidpoint(myPoint, myPoint);
		myPoint getLeftShoulder() { return JOINT_SHOULDER_LEFT; }
		myPoint getRightShoulder() { return JOINT_SHOULDER_RIGHT; }

};

void myPerson::updateJoints(myPoint head, myPoint lShoulder, myPoint rShoulder, myPoint lHand, myPoint rHand, myPoint midSpine) {
	JOINT_HEAD = head;
	JOINT_SHOULDER_LEFT = lShoulder;
	JOINT_SHOULDER_RIGHT = rShoulder;
	JOINT_HAND_LEFT = lHand;
	JOINT_HAND_RIGHT = rHand;
	JOINT_SPINE_MID = midSpine;

	shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);
	leftArmLength = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT);
	rightArmLength = calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT);
	torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID);

}

void myPerson::printPerson() {
	printf("Printing person%d:\n");
	printf("  Joint Locations:\n");
	printf("    JOINT_HEAD:           "); JOINT_HEAD.printPoint();
	printf("    JOINT_SHOULDER_LEFT:  "); JOINT_SHOULDER_LEFT.printPoint();
	printf("    JOINT_SHOULDER_RIGHT: "); JOINT_SHOULDER_RIGHT.printPoint();
	printf("    JOINT_HAND_LEFT:      "); JOINT_HAND_LEFT.printPoint();
	printf("    JOINT_HAND_RIGHT:     "); JOINT_HAND_RIGHT.printPoint();
	printf("    JOINT_SPINE_MID:      "); JOINT_SPINE_MID.printPoint();
	printf("  Calculated Features:\n");
	printf("    shoulderDistance:     %.2f\n", shoulderDistance);
	printf("    leftArmLength:        %.2f\n", leftArmLength);
	printf("    rightArmLength:       %.2f\n", rightArmLength);
	printf("    torsoHeight:          %.2f\n", torsoHeight);


}

double myPerson::calculateDistance(myPoint point1, myPoint point2) { //order of params doesn't matter
	double difX = point1.getWorldX() - point2.getWorldX();
	double difY = point1.getWorldY() - point2.getWorldY();
	double difZ = point1.getWorldZ() - point2.getWorldZ();
	double sumXYZ = pow(difX, 2) + pow(difY, 2) + pow(difZ, 2);
	return sqrt(sumXYZ);
}

myPoint myPerson::calculateMidpoint(myPoint point1, myPoint point2) { //order of params doesn't matter
	double difX = fabs(point1.getWorldX() - point2.getWorldX()); //float absolute value function
	double difY = fabs(point1.getWorldY() - point2.getWorldY());
	double difZ = fabs(point1.getWorldZ() - point2.getWorldZ());
	double finalX, finalY, finalZ;
	if (point1.getWorldX() < point2.getWorldX()) { //use smaller x value, add to it
		finalX = point1.getWorldX() + (difX / 2);
	}
	else {
		finalX = point2.getWorldX() + (difX / 2);
	}
	if (point1.getWorldY() < point2.getWorldY()) { //use smaller y value, add to it
		finalY = point1.getWorldY() + (difY / 2);
	}
	else {
		finalY = point2.getWorldY() + (difY / 2);
	}
	if (point1.getWorldZ() < point2.getWorldZ()) { //use smaller z value, add to it
		finalZ = point1.getWorldZ() + (difZ / 2);
	}
	else {
		finalZ = point2.getWorldZ() + (difZ / 2);
	}
	myPoint toReturn(finalX, finalY, finalZ);
	return toReturn;
}