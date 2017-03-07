#include "myPerson.h"
#include <conio.h>


int main() {
	printf("Hello World!!\n");
	myPoint testPoint(1, 2, 3);
	testPoint.printPoint();

	//myPerson testPerson;
	//testPerson.printPerson();
	myPoint newHead(25, 25, 0);
	myPoint newLeftShoulder(15, 5, 0);
	myPoint newRightShoulder(35, 5, 0);
	myPoint newRightHand(30, 0, 0);
	myPoint newLeftHand(20, 0, 0);
	myPoint newSpineMid(25, 0, 0);
	myPerson testPerson(newHead, newLeftShoulder, newRightShoulder, newLeftHand, newRightHand, newSpineMid);
	testPerson.printPerson();

	myPerson testPerson2;
	testPerson2.printPerson();
	printf("Press Q to quit...");
	while (true) {
		if (_kbhit()) { // Break loop
			int c = _getch() & 255;
			if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
		}
	}
}