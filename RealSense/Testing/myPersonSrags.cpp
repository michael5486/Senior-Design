#include "myPersonSrags.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>




int main() {
	printf("Hello World!!\n");
	myPoint testPoint(1,2,3);
	//testPoint.printPoint();
	
	myPerson testPerson;
	//testPerson.printPerson();
	myPoint newHead(25,25,0);
	myPoint newSpineMid(25, 10, 0);
	myPoint newLeftShoulder(15,15,0);
	myPoint newRightShoulder(35,15,0);
	myPoint newRightHand(30,5,0);
	myPoint newLeftHand(20,5,0);
	testPerson.updateJoints(newHead, newSpineMid, newLeftShoulder, newRightShoulder, newLeftHand, newRightHand);
	testPerson.printPerson();

	myPerson testPerson2;
	//testPerson2.printPerson();
	myPoint newHead2(25,25,0);
	myPoint newSpineMid2(25, 10, 0);
	myPoint newLeftShoulder2(15,15,0);
	myPoint newRightShoulder2(35,15,0);
	myPoint newRightHand2(30,5,0);
	myPoint newLeftHand2(20,5,0);
	testPerson2.updateJoints(newHead2, newSpineMid2, newLeftShoulder2, newRightShoulder2, newLeftHand2, newRightHand2);
	testPerson2.printPerson();

	//they are the same person, should test return 100. it works :)
	compareTorsoRatio(testPerson, testPerson2);

	myPerson testPerson3;
	//testPerson2.printPerson();
	myPoint newHead3(25,25,0);
	myPoint newSpineMid3(25, 9.5, 0);
	myPoint newLeftShoulder3(15,15,0);
	myPoint newRightShoulder3(35,15,0);
	myPoint newRightHand3(30,5,0);
	myPoint newLeftHand3(20,5,0);
	testPerson3.updateJoints(newHead3, newSpineMid3, newLeftShoulder3, newRightShoulder3, newLeftHand3, newRightHand3);
	testPerson3.printPerson();

	compareTorsoRatio(testPerson, testPerson3);

}
