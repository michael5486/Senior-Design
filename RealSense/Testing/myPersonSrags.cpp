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
	myPoint newSpineMid(25, 10, 0);
	myPoint newLeftShoulder(15,15,0);
	myPoint newRightShoulder(35,15,0);
	myPoint newRightHand(30,5,0);
	myPoint newLeftHand(20,5,0);
	testPerson.updateJoints(newHead, newSpineMid, newLeftShoulder, newRightShoulder, newLeftHand, newRightHand);
	testPerson.printPerson();
}
