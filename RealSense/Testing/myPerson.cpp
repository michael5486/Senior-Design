#include "myPerson.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>




int main() {
	printf("Hello World!!\n");
	myPoint testPoint(1,2,3);
	testPoint.printPoint();
	
	myPerson testPerson;
	testPerson.printPerson();
	myPoint newHead(25,25,0);
	myPoint newLeftShoulder(15,5,0);
	myPoint newRightShoulder(35,5,0);
	myPoint newRightFoot(30,0,0);
	myPoint newLeftFoot(20,0,0);
	testPerson.updateJoints(newHead, newLeftShoulder, newRightShoulder, newLeftFoot, newRightFoot);
	testPerson.printPerson();
	myPoint midFeet = testPerson.calculateMidpoint(testPerson.getRightFoot(), testPerson.getLeftFoot());
	midFeet.printPoint();
}
