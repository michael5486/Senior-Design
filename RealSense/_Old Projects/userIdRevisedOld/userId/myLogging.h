/* Header file for all my our methods related to logging information to external files */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

#define COUNT_WIDTH 15
#define COLUMN_WIDTH 35
#define VECTOR_WIDTH 6

using namespace std;

/* Externally defined variables */
extern int timeCounter;
extern ofstream jointLog;
extern ofstream torsoLog;
extern ofstream leftArmLog;
extern ofstream rightArmLog;


/* Method declarations */
void createJointLogFile(string fileName);
void createVectorLogFile(string fileName, string feature, ofstream& measurement);
string pointToString(myPoint point);
void printToJointLog(myPerson person);
void printToVectorLog(vector<double> vect, ofstream& measurement);
boolean isJointInfoValid(PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints);



void createJointLogFile(string string) {
	jointLog.open(string);

	jointLog << "\n";
	jointLog << "Beginning joint data output...";
	jointLog << "\n\n\n";

	jointLog << left << setw(COUNT_WIDTH) << setfill(separator) << "Time";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HEAD";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SHOULDER_LEFT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SHOULDER_RIGHT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HAND_LEFT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HAND_RIGHT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SPINE_MID";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_CENTER_MASS";

	jointLog << "\n";
}

/* Creates the file to store our vector data */
void createVectorLogFile(string fileName, string featureType, ofstream& measurement) {
	measurement.open(fileName);
	measurement << "\n";
	measurement << "Beginning " << featureType << " data output...";
	measurement << "\n\n\n";
}

/* Takes a myPerson as input, prints it to the joint log file */
void printToJointLog(myPerson newPerson) {
	jointLog << left << setw(COUNT_WIDTH) << setfill(separator) << timeCounter;
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getHead());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getLeftShoulder());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getRightShoulder());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getSpineMid());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getCenterMass());
	jointLog << "\n";
}

/* Prints the entire vector onto one line in the vector log file */
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


}

/* Returns x and y coordinates of images, z coordinate from world */
string pointToString(myPoint point) {
	//point.printPoint();
	stringstream ss;

	ss << left << setprecision(5) << setw(5) << point.getImageX();
	ss << left << ", ";
	ss << left << setprecision(5) << setw(5) << point.getImageY();
	ss << left << ", ";
	ss << left << setprecision(5) << setw(5) << point.getWorldZ();
	return ss.str(); //converts stringStream to a string
}

myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* personData) {
	assert(personData != NULL);
	PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();
	/* Initializes null person */
	myPerson newPerson;

	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	personJoints->QueryJoints(joints);

	PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
	myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y * 1000);

	/* Joint info invalid, returns null user */
	if (isJointInfoValid(joints) == false) {
		//printf("Converstion unsuccessful, outputting null person...\n");
	}
	/* Joint info is valid, changes newPerson to have valid joint data */
	else {
		//printf("Conversion successful, outputting to log...\n");
		//myPoint leftHand(joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
		//myPoint rightHand(joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
		myPoint head(joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft(joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight(joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid(joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
		newPerson.changeJoints(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);
	}
	timeCounter++;
	delete[] joints;
	return newPerson;

}

/* Checks to see if gathered joint info is valid data */
boolean isJointInfoValid(PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints) {
	/* Not valid data, return false */
	if (joints[2].jointType != 10 || joints[3].jointType != 19 || joints[4].jointType != 16 || joints[5].jointType != 17) {
		return false;
		printf("Garbage data...\n");
	}
	if (joints[2].image.x == 0 && joints[2].image.y == 0) { //no image coordinates for left shoulder, skips initialization
		printf("Invalid head...\n");
		return false;
	}
	if (joints[3].image.x == 0 && joints[3].image.y == 0) { //no image coordinates for right shoulder, skips initialization
		printf("Invalid spine mid...\n");
		return false;
	}
	if (joints[4].image.x == 0 && joints[4].image.y == 0) { //no image coordinates for left shoulder, skips initialization
		printf("Invalid left shoulder...\n");
		return false;
	}
	if (joints[5].image.x == 0 && joints[5].image.y == 0) { //no image coordinates for right shoulder, skips initialization
		printf("Invalid right shoulder...\n");
		return false;
	}
	/* All tested issues passed, joint info is valid */
	return true;
}