#pragma once
#include <math.h>
#include "myPoint.h"

#define e 2.718 //mathematical constant
#define RATIO_EXP_DECAY 2.31 //experimentally generated value
#define VAL_EXP_DECAY 0.15


int personCounter = 0; //global variable, increments for each new person constructed

class myPerson {
private:
	int personID;
	double torsoHeight, armLength;
	myPoint JOINT_HAND_LEFT;
	myPoint JOINT_HAND_RIGHT;
	myPoint JOINT_HEAD;
	myPoint JOINT_SPINE_MID;
	myPoint JOINT_SHOULDER_LEFT;
	myPoint JOINT_SHOULDER_RIGHT;

public:
	//what is this?
	myPerson() : JOINT_HEAD(), JOINT_SHOULDER_LEFT(), JOINT_SHOULDER_RIGHT(), JOINT_HAND_LEFT(),
		JOINT_HAND_RIGHT(), JOINT_SPINE_MID() {
		personID = personCounter++;
		torsoHeight = 0;
		armLength = 0;
	}
	void printPerson();
	void updateJoints(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
	double calculateDistance(myPoint, myPoint);
	//myPoint calculateMidpoint(myPoint, myPoint);
	double getTorso() { return torsoHeight; }
	double getArmLength() { return armLength; }
	// Center of area calc already part of person tracking module

};

void myPerson::updateJoints(myPoint newHead, myPoint newSpineMid, myPoint newShoulderLeft, myPoint newShoulderRight, myPoint newHandLeft, myPoint newHandRight) {
	JOINT_HEAD = newHead;
	JOINT_SPINE_MID = newSpineMid;
	JOINT_SHOULDER_LEFT = newShoulderLeft;
	JOINT_SHOULDER_RIGHT = newShoulderRight;
	JOINT_HAND_LEFT = newHandLeft;
	JOINT_HAND_RIGHT = newHandRight;

	//shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT);
	double tmp = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT);
	double tmp2 = calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT);
	armLength = (tmp + tmp2) / 2;
	torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID);



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
	printf("    ArmLength:            %.2f\n", armLength);
	printf("    UpperBodyHeight:      %.2f\n", torsoHeight);


}

double myPerson::calculateDistance(myPoint point1, myPoint point2) { //order of params doesn't matter
	double difX = point1.getX() - point2.getX();
	double difY = point1.getY() - point2.getY();
	double difZ = point1.getZ() - point2.getZ();
	double sumXYZ = pow(difX, 2) + pow(difY, 2) + pow(difZ, 2);
	return sqrt(sumXYZ);
}

/*
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
*/

double compareTorsoRatio(myPerson person1, myPerson person2) {
	//compares person2 against person1...order doesn't matter
	//returns value 100 if ratios are identical, with expnentially decreasing values as they get more different
	// y  = 100e^(-2.31*x)
	// x = | ratio1 - ratio 2 |	
	double ratio1 = person1.getTorso() / person1.getArmLength();
	double ratio2 = person2.getTorso() / person2.getArmLength();

	double x = fabs(ratio1 - ratio2);

	double neg = -1 * RATIO_EXP_DECAY * x;

	double eToPower = pow(e, neg);

	//printf("e^power: %f\n", eToPower);

	double y = 100 * eToPower;

	printf("Ratio Similarity = %f\n", y);

	return y;
}

double compareTorsoAndArmLengths(myPerson person1, myPerson person2) {
	//compares the actual values of torso and arm lengths...order doesn't matter
	//returns value between 0 and 100, 100 being that values are identical	
	double arm1 = person1.getArmLength();
	double arm2 = person2.getArmLength();

	double torso1 = person1.getTorso();
	double torso2 = person2.getTorso();

	double x1 = fabs(arm1 - arm2);
	double neg = -1 * VAL_EXP_DECAY * x1;

	double eToPower = pow(e, neg);

	//printf("e^power: %f\n", eToPower);

	double y1 = 100 * eToPower;

	printf("Arm Similarity = %f\n", y1);


	double x2 = fabs(torso1 - torso2);
	neg = -1 * VAL_EXP_DECAY * x2;
	eToPower = pow(e, neg);
	//printf("e^power: %f\n", eToPower);
	double y2 = 100 * eToPower;

	printf("Torso Similarity = %f\n", y2);

	//calculate average of torso and arm scores
	printf("Combined Similarity: %f\n", (y1 + y2) / 2);

	return (y1 + y2) / 2;

}

double comparePeople(myPerson person1, myPerson person2) {
	//calculates the similarity between two people using all defined methods

	//may want to place a greater value on features we determine are more useful	

	double num1 = compareTorsoRatio(person1, person2);
	double num2 = compareTorsoAndArmLengths(person1, person2);

	//calculating the average between the two for now
	double toReturn = (num1 + num2) / 2;

	printf("Final Similarity = %f\n", toReturn);

	return toReturn;


}
