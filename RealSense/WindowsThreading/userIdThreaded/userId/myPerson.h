#include "myPoint.h"
#include <math.h>
#include <vector>
#include <algorithm> //for sorting vectors
#include <iomanip> //for log formatting


#define e 2.718 //mathematical constant
#define RATIO_EXP_DECAY 2.31 //experimentally generated value
#define VAL_EXP_DECAY 0.15

#define INITIALIZE_COUNT_MAX 10 //number of iterations before target is initialized


using namespace std; //needed for using the vector

int personCounter = 0; //global variable, increments for each new person constructed

/* Function declarations */
double findMedian(vector<double>);
double findMedianForZ(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
double convertToInches(double, double);



/* Needed for logging purposes */
char separator = ' ';
#define VECTOR_WIDTH 6


//void printToVectorLog(vector<double> vect, ofstream& measurement);


/* Variable declarations for logging purposes 
ofstream zAxisLog;*/


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
		myPoint JOINT_CENTER_MASS;

		/* Vectors used for keeping track of user history */
		vector<double> torsoHeightVector;
		vector<double> leftArmVector;
		vector<double> rightArmVector;
		vector<double> shoulderVector;
		//vector<double> zValues;

	public: 
		//Primary Constructor
		myPerson(myPoint head, myPoint lShoulder, myPoint rShoulder, 
				myPoint lHand, myPoint rHand, myPoint midSpine, myPoint cMass) {
			JOINT_HEAD = head;
			JOINT_SHOULDER_LEFT = lShoulder;
			JOINT_SHOULDER_RIGHT = rShoulder;
			JOINT_HAND_LEFT = lHand;
			JOINT_HAND_RIGHT = rHand;
			JOINT_SPINE_MID = midSpine;
			JOINT_CENTER_MASS = cMass;
			personID = personCounter++;

			/* Finds the median of the z values */
			double medianZ = findMedianForZ(head, lShoulder, rShoulder, lHand, rHand, midSpine, cMass);

			shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT, medianZ);
			leftArmLength = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT, medianZ);
			rightArmLength = calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT, medianZ);
			torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID, medianZ);
		}
		//Default Constructor, sets all members to 0
		myPerson() {
			myPoint head, lShoulder, rShoulder, lHand, rHand, midSpine, cMass;
			JOINT_HEAD = head;
			JOINT_SHOULDER_LEFT = lShoulder;
			JOINT_SHOULDER_RIGHT = rShoulder;
			JOINT_HAND_LEFT = lHand;
			JOINT_HAND_RIGHT = rHand;
			JOINT_SPINE_MID = midSpine;
			JOINT_CENTER_MASS = cMass;
			
			personID = personCounter++;
			shoulderDistance = 0;
			leftArmLength = 0;
			rightArmLength = 0;
			torsoHeight = 0;


		}
		/* method declarations */
		void printPerson();
		void changeJoints(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
		void updatePerson(myPoint, myPoint, myPoint, myPoint, myPoint, myPoint, myPoint);
		double calculateDistance(myPoint, myPoint, double);
		myPoint calculateMidpoint(myPoint, myPoint);
		/* accessor methods */
		double getLeftArmLength() { return leftArmLength; }
		double getRightArmLength() { return rightArmLength; }
		double getTorso() { return torsoHeight; }
		double getShoulderDistance() { return shoulderDistance; }
		int getInitializeCount() { return initializeCount; }
	//	void initializeVectorLog(ofstream&);

		myPoint getHead() { return JOINT_HEAD; }
		myPoint getLeftShoulder() { return JOINT_SHOULDER_LEFT; }
		myPoint getRightShoulder() { return JOINT_SHOULDER_RIGHT; }
		myPoint getLeftHand() { return JOINT_HAND_LEFT; }
		myPoint getRightHand() { return JOINT_HAND_RIGHT; }
		myPoint getSpineMid() { return JOINT_SPINE_MID; }
		myPoint getCenterMass() { return JOINT_CENTER_MASS; }

		vector<double> getTorsoVector() { return torsoHeightVector; }
		vector<double> getLeftArmVector() { return leftArmVector; }
		vector<double> getRightArmVector() { return rightArmVector; }
		double getMedianTorsoHeight();
		double getMedianLeftArmLength();
		double getMedianRightArmLength();
		double getMedianShoulderDistance();
		double getArmLength();
};

/* Changes the joints for a person and the respective distance calculations */
void myPerson::changeJoints(myPoint head, myPoint lShoulder, myPoint rShoulder, myPoint lHand, myPoint rHand, myPoint midSpine, myPoint cMass) {
	JOINT_HEAD = head;
	JOINT_SHOULDER_LEFT = lShoulder;
	JOINT_SHOULDER_RIGHT = rShoulder;
	JOINT_HAND_LEFT = lHand;
	JOINT_HAND_RIGHT = rHand;
	JOINT_SPINE_MID = midSpine;
	JOINT_CENTER_MASS = cMass;

	double medianZ = findMedianForZ(head, lShoulder, rShoulder, lHand, rHand, midSpine, cMass);

	shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT, medianZ);
	leftArmLength = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT, medianZ);
	rightArmLength = calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT, medianZ);
	torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID, medianZ);

}

/* Updates the joints of the person, but recalculates */
void myPerson::updatePerson(myPoint head, myPoint lShoulder, myPoint rShoulder, myPoint lHand, myPoint rHand, myPoint midSpine, myPoint cMass) {
	JOINT_HEAD = head;
	JOINT_SHOULDER_LEFT = lShoulder;
	JOINT_SHOULDER_RIGHT = rShoulder;
	JOINT_HAND_LEFT = lHand;
	JOINT_HAND_RIGHT = rHand;
	JOINT_SPINE_MID = midSpine;
	JOINT_CENTER_MASS = cMass;

	double medianZ = findMedianForZ(head, lShoulder, rShoulder, lHand, rHand, midSpine, cMass);

	/* Calculate the torsoHeight in current frame, adds to torsoHeightVector */
	torsoHeight = calculateDistance(JOINT_HEAD, JOINT_SPINE_MID, medianZ);
	torsoHeightVector.push_back(torsoHeight);

	/* Same for leftArm and rightArm */
	leftArmLength = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_HAND_LEFT, medianZ);
	leftArmVector.push_back(leftArmLength);
	rightArmLength = calculateDistance(JOINT_SHOULDER_RIGHT, JOINT_HAND_RIGHT, medianZ);
	rightArmVector.push_back(rightArmLength);
	shoulderDistance = calculateDistance(JOINT_SHOULDER_LEFT, JOINT_SHOULDER_RIGHT, medianZ);
	shoulderVector.push_back(shoulderDistance);

}

void myPerson::printPerson() {
	printf("Printing person%d:\n", personID);
	printf("  Joint Locations:\n");
	printf("    JOINT_HEAD:           "); JOINT_HEAD.printPoint();
	printf("    JOINT_SHOULDER_LEFT:  "); JOINT_SHOULDER_LEFT.printPoint();
	printf("    JOINT_SHOULDER_RIGHT: "); JOINT_SHOULDER_RIGHT.printPoint();
	printf("    JOINT_HAND_LEFT:      "); JOINT_HAND_LEFT.printPoint();
	printf("    JOINT_HAND_RIGHT:     "); JOINT_HAND_RIGHT.printPoint();
	printf("    JOINT_SPINE_MID:      "); JOINT_SPINE_MID.printPoint();
	printf("    JOINT_CENTER_MASS:    "); JOINT_CENTER_MASS.printPoint();
	printf("  Calculated Features:\n");
	printf("    shoulderDistance:     %.2f\n", shoulderDistance);
	printf("    leftArmLength:        %.2f\n", leftArmLength);
	printf("    rightArmLength:       %.2f\n", rightArmLength);
	printf("    torsoHeight:          %.2f\n", torsoHeight);


}

/* We are assumming all joints are on the same z plane. Will need to implement the trig here */
double myPerson::calculateDistance(myPoint point1, myPoint point2, double medianZ) { //order of params doesn't matter
	//double difX = point1.getWorldX() - point2.getWorldX();
	//double difY = point1.getWorldY() - point2.getWorldY();
	//double difZ = point1.getWorldZ() - point2.getWorldZ();
	//double sumXYZ = pow(difX, 2) + pow(difY, 2) + pow(difZ, 2);
	double difPixelsX = point1.getImageX() - point2.getImageX();
	double difInchesX = convertToInches(difPixelsX, medianZ);
	double difPixelsY = point1.getImageY() - point2.getImageY();
	double difInchesY = convertToInches(difPixelsY, medianZ);
	double sumXY = pow(difInchesX, 2) + pow(difInchesY, 2);
	return sqrt(sumXY);
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
//lol this method is dumb
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

double myPerson::getMedianTorsoHeight() {
	return findMedian(torsoHeightVector);
}
double myPerson::getMedianLeftArmLength() {
	return findMedian(leftArmVector);
}
double myPerson::getMedianRightArmLength() {
	return findMedian(rightArmVector);
}
double myPerson::getMedianShoulderDistance() {
	return findMedian(shoulderVector);
}
/* End of inherited functions needed for use in other classes. All ones below this are only used in myPerson.h */


/* Compares person1 against target user, assuming target user is second parameter
   returns value 100 if ratios are identical, with expnentially decreasing values as they get more different
   y  = 100e^(-2.31*x)
   x = | ratio1 - ratio 2 | */
double compareTorsoRatio(myPerson person1, myPerson targetUser) {
	
	//double ratio1 = person1.getTorso() / person1.getArmLength();
	//double ratio2 = person2.getTorso() / person2.getArmLength();

	double ratio1 = person1.getTorso() / person1.getShoulderDistance();
	//printf("person1Torso = %f person1Shoulder = %f\n", person1.getTorso(), person1.getShoulderDistance());
	double ratio2 = targetUser.getMedianTorsoHeight() / targetUser.getMedianShoulderDistance();
	//printf("targetUserTorso = %f targetUserShoulder = %f\n", targetUser.getMedianTorsoHeight(), targetUser.getMedianShoulderDistance());


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

	//double arm1 = person1.getArmLength();
	//double arm2 = person2.getArmLength();
	double shoulder1 = person1.getShoulderDistance();
	double shoulder2 = person2.getShoulderDistance();


	double torso1 = person1.getTorso();
	double torso2 = person2.getTorso();

	double x1 = fabs(shoulder1 - shoulder2);
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

/* Iterates through and prints the vector */
void printVector(vector<double> vect) {
	int i = 0;
	for (vector<double>::iterator it = vect.begin(); it != vect.end(); it++) {
		printf("%d  |  %f\n", i, *it);
		i++;
	}
}

/* Finds the median location of the inputted vector and returns the respective value at that location */
double findMedian(vector<double> vect) {
	sort(vect.begin(), vect.end());
	//printVector(vect);

	int size = vect.size();
	//printf("size= %d", size);

	if (vect.empty()) { //returns -1 if vect is empty
		return -1;
	}
	else if (size % 2 == 0) { //if vect contains even number of items
		int medianLoc = size / 2;
		//printf("medianLoc = %d", medianLoc);
		double val1 = vect[medianLoc];
		double val2 = vect[medianLoc - 1];
		//printf("medianLoc= %d  medianLoc-1 = %d\n", medianLoc, medianLoc-1);
		return (val1 + val2) / 2;

	}
	else { //vect contains odd # of values
		double medianLoc = size / 2;
		//printf("medianLoc = %d", medianLoc);
		medianLoc = floor(medianLoc);
		//	printf("medianLoc = %d\n", medianLoc);
		return vect[medianLoc];
	}
}

/* Find the median of the z values */
double findMedianForZ(myPoint head, myPoint lShoulder, myPoint rShoulder, myPoint lHand, myPoint rHand, myPoint spineMid, myPoint cMass) {
	vector<double> zValues;
	zValues.push_back(head.getWorldZ());
	zValues.push_back(lShoulder.getWorldZ());
	zValues.push_back(rShoulder.getWorldZ());
	zValues.push_back(lHand.getWorldZ());
	zValues.push_back(rHand.getWorldZ());
	zValues.push_back(spineMid.getWorldZ());
	zValues.push_back(cMass.getWorldZ() * 1000); //for some reason, cMass is reported in different units
	/*cout << "\n  Unsorted:  ";
	for (vector<double>::iterator it = zValues.begin(); it != zValues.end(); it++) {
		cout << left << setprecision(4) << setw(VECTOR_WIDTH) << setfill(separator) << *it;
	}
//	printToVectorLog(zValues, zAxisLog);
	cout << "\n  Sorted:  ";
	sort(zValues.begin(), zValues.end());
	for (vector<double>::iterator it = zValues.begin(); it != zValues.end(); it++) {
		cout << left << setprecision(4) << setw(VECTOR_WIDTH) << setfill(separator) << *it;
	}
	cout << "     median = " << zValues[3];
	cout << "\n";*/
	return zValues[3];
}


/* Input distance between two points in image, uses Z axis distance to scale pixel difference to inch difference.
   Created using experimentally generated values */
double convertToInches(double zAxisInRSU, double pixelDistance) {

	double inchesPerPixel = 0.000119 * zAxisInRSU + 0.00308;
	return inchesPerPixel * pixelDistance;

}


/* Prints the entire vector onto one line in the vector log file 
void printToVectorLog(vector<double> vect, ofstream& measurement) {

	measurement << "Unsorted: ";
	for (vector<double>::iterator it = vect.begin(); it != vect.end(); it++) {
		measurement << left << setprecision(4) << setw(VECTOR_WIDTH) << setfill(separator) << *it;
	}
	measurement << "\n";
	measurement << "Sorted:   ";
	sort(vect.begin(), vect.end());
	for (vector<double>::iterator it = vect.begin(); it != vect.end(); it++) {
		measurement << left << setprecision(4) << setw(VECTOR_WIDTH) << setfill(separator) << *it;
	}
	measurement << "\n median: ";
	measurement << findMedian(vect);
	measurement << "\n";


}*/