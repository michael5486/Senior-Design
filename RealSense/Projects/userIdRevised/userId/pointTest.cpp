#include <conio.h>
#include "myPoint.h"

int main() {
	printf("Hello World\n");
	printf("TestPoint1");
	myPoint testPoint(1, 2, 3);
	testPoint.printPoint();
	printf("TestPoint1");
	testPoint.updateValues(4, 5, 6);
	testPoint.printPoint();
	myPoint testPoint2;
	printf("testPoint2");
	testPoint2.printPoint();

	printf("Press Q to quit...");
	while (true) {
		if (_kbhit()) { // Break loop
			int c = _getch() & 255;
			if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
		}
	}

}