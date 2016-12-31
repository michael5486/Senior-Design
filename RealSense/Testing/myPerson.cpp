#include "myPoint.h"
#include "myPerson.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>


/*myPerson createPerson () {

	myPerson newPerson;
	newPerson.JOINT_HEAD = createPoint(0,0,0);
	newPerson.JOINT_SHOULDER_LEFT = createPoint(0,0,0);
	newPerson.JOINT_SHOULDER_RIGHT = createPoint(0,0,0);
	newPerson.JOINT_HAND_RIGHT = createPoint(0,0,0);
	newPerson.JOINT_HIP_LEFT = createPoint(0,0,0);
	newPerson.JOINT_HIP_RIGHT = createPoint(0,0,0);
	newPerson.JOINT_FOOT_RIGHT = createPoint(0,0,0);
	return newPerson;

}*/

myPerson::myPerson(): test(0) {

	//JOINT_HEAD = head;
	/*, JOINT_SHOULDER_LEFT(myPoint leftShoulder), JOINT_SHOULDER_RIGHT(myPoint rightShoulder),
	JOINT_HAND_RIGHT(myPoint rightHand), JOINT_HIP_LEFT(myPoint leftHip), JOINT_HIP_RIGHT(myPoint rightHip), JOINT_FOOT_RIGHT(myPoint rightHip)*/

}	

void myPerson::printPerson () {

	//printf("JOINT_HEAD: "); JOINT_HEAD.printPoint();
	/*printf("JOINT_SHOULDER_LEFT: "); myprintPoint(JOINT_SHOULDER_LEFT);
	printf("JOINT_SHOULDER_RIGHT: "); printPoint(JOINT_SHOULDER_RIGHT);
	printf("JOINT_HAND_RIGHT: "); printPoint(JOINT_HAND_RIGHT);
	printf("JOINT_HIP_LEFT: "); printPoint(JOINT_HIP_LEFT);
	printf("JOINT_HIP_RIGHT: "); printPoint(JOINT_HIP_RIGHT);
	printf("JOINT_HAND_RIGHT: "); printPoint(JOINT_HAND_RIGHT);*/

}


/*double calculateDistance(myPoint point1, myPoint point2) {
	double difX = point1.x - point2.x;
	double difY = point1.y - point2.y;
	double difZ = point1.z - point2.z;
	double sumXYZ = pow(difX, 2) + pow(difY, 2) + pow(difZ, 2);
	return sqrt(sumXYZ);
}*/

/*int main() {
	printf("Hello World!!\n");
	myPerson testing;
	//myPoint testPoint;
	//testPoint.updateValues(10, 10, 0);
	//orig.printPoint();
	//testPoint.printPoint();


}*.
