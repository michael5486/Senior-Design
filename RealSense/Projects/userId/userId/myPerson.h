#include "myPoint.h"
#include <math.h>

#define e 2.718 //mathematical constant
#define RATIO_EXP_DECAY 2.31 //experimentally generated value
#define VAL_EXP_DECAY 0.15

#define INITIALIZE_COUNT_MAX 10 //number of iterations before target is initialized

int personCounter = 0; //global variable, increments for each new person constructed

class myPerson{
	private:
		int initializeCount;
		int personID; //unneccessary using my method
		double shoulderDistance, leftArmLength, rightArmLength, torsoHeight; //raw shoulder values
		//double shoulderDistanceSum, leftArmLengthSum, rightArmLengthSum, rightArmLengthSum; //continuosly updated distance values. Refined over multiple iterations
		
		myPoint JOINT_HEAD;
		myPoint JOINT_SHOULDER_LEFT;
		myPoint JOINT_SHOULDER_RIGHT;
		myPoint JOINT_HAND_LEFT;
		myPoint JOINT_HAND_RIGHT;
		myPoint JOINT_SPINE_MID;
	public: 
		//Primary Constructor
		myPerson(myPoint head, myPoint lShoulder, myPoint rShoulder, 
				myPoint lHand, myPoint rHand, myPoint midSpine) {
			JOINT_HEAD = head;
			JOINT_SHOULDER_LEFT = lShoulder;
			JOINT_SHOULDER_RIGHT = rShoulder;
			JOINT_HAND_LEFT = lHand;
			JOINT_HAND_RIGHT = rHand;
			JOINT_SPINE_MID = midSpine;
			personID = personCounter++;
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
			
			personID = personCounter++;
			shoulderDistance = 0;
			leftArmLength = 0;
			rightArmLength = 0;
			torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID);

		}
		void printPerson();
		void changeJoints(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
		double calculateDistance(myPoint, myPoint);
		myPoint calculateMidpoint(myPoint, myPoint);

		double getLeftArmLength() { return leftArmLength; }
		double getRightArmLength() { return rightArmLength; }
		double getTorso() { return torsoHeight; }
		double getArmLength();
		int getInitializeCount() { return initializeCount; }

		myPoint getHead() { return JOINT_HEAD; }
		myPoint getLeftShoulder() { return JOINT_SHOULDER_LEFT; }
		myPoint getRightShoulder() { return JOINT_SHOULDER_RIGHT; }
		myPoint getLeftHand() { return JOINT_HAND_LEFT; }
		myPoint getRightHand() { return JOINT_HAND_LEFT; }
		myPoint getSpineMid() { return JOINT_SPINE_MID; }
};

/* Changes the joints for a person and the respective distance calculations */
void myPerson::changeJoints(myPoint head, myPoint lShoulder, myPoint rShoulder, myPoint lHand, myPoint rHand, myPoint midSpine) {
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

/* Updates the joints of the person, but recalculates
void myPerson::updatePerson(myPoint head, myPoint lShoulder, myPoint rShoulder, myPoint lHand, myPoint rHand, myPoint midSpine, int initializeCount) {

}*/

void myPerson::printPerson() {
	printf("Printing person%d:\n", personID);
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

/* Returns the average of rightArmLength and leftArmLength */
double myPerson::getArmLength() {
	double leftArmLength = this->getLeftArmLength();
	double rightArmLength = this->getRightArmLength();
	//Are these issues we will have to worry about? I understand the concept but I don't see how making it 0 helps
	//Also dont use shitty variable names...there will be a lot of code and while ral might make sense now it wont in a couple weeks
	if (leftArmLength == 0 && rightArmLength == 0) { 
		printf("Arm lengths not reliable");
		return 0.0;
	}
	else if (leftArmLength == 0) {
		return rightArmLength;
	}
	else if (rightArmLength == 0) {
		return leftArmLength;
	}
	else {
		return (rightArmLength + leftArmLength) / 2;
	}
}


/* Compares person1 against person2. Order doesn't matter 
   returns value 100 if ratios are identical, with expnentially decreasing values as they get more different
   y  = 100e^(-2.31*x)
   x = | ratio1 - ratio 2 | */
double compareTorsoRatio(myPerson person1, myPerson person2) {
	
	double ratio1 = person1.getTorso() / person1.getArmLength();
	double ratio2 = person2.getTorso() / person2.getArmLength();

	double x = fabs(ratio1 - ratio2);

	double neg = -1 * RATIO_EXP_DECAY * x;

	double eToPower = pow(e, neg);
	double y = 100 * eToPower;

	//printf("Ratio Similarity = %f\n", y);

	return y;
}

/* Compares the actual values of torso and arm lengths. Order doesn't matter
   returns value between 0 and 100. 100 means that values are identical	*/
double compareTorsoAndArmLengths(myPerson person1, myPerson person2) {

	double arm1 = person1.getArmLength();
	double arm2 = person2.getArmLength();

	double torso1 = person1.getTorso();
	double torso2 = person2.getTorso();

	double x1 = fabs(arm1 - arm2);
	double neg = -1 * VAL_EXP_DECAY * x1;

	double eToPower = pow(e, neg);

	//printf("e^power: %f\n", eToPower);

	double y1 = 100 * eToPower;

	//printf("Arm Similarity = %f\n", y1);


	double x2 = fabs(torso1 - torso2);
	neg = -1 * VAL_EXP_DECAY * x2;
	eToPower = pow(e, neg);
	//printf("e^power: %f\n", eToPower);
	double y2 = 100 * eToPower;

	//printf("Torso Similarity = %f\n", y2);

	//calculate average of torso and arm scores
	//printf("Combined Similarity: %f\n", (y1 + y2) / 2);

	return (y1 + y2) / 2;
}