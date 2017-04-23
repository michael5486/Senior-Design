#include "myPerson.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>




int main() {
	printf("Hello World!!\n");
	myPoint testPoint(1,2,3);
	testPoint.printPoint();
	
	//myPerson testPerson;
	//testPerson.printPerson();
	myPoint newHead(25,25,0);
	myPoint newLeftShoulder(15,5,0);
	myPoint newRightShoulder(35,5,0);
	myPoint newRightHand(30,0,0);
	myPoint newLeftHand(20,0,0);
	myPoint newSpineMid(25, 0, 0);
	myPerson testPerson(newHead, newLeftShoulder, newRightShoulder, newLeftHand, newRightHand, newSpineMid);
	testPerson.printPerson();

	myPerson testPerson2;
	testPerson2.printPerson();
}
