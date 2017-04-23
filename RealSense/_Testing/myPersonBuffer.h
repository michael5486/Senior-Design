#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "myPerson.h"

#define maxCount = 10;

struct measurements {
	double torsoHeight;
	double shoulderWidth;
};

class myPersonBuffer {

	private:
		int count;

	public:
	//primary constructor
		myPersonBuffer() {
			count = 0;

		}

	void addPerson();
	void add(int, myBuffer);
	void add(int, measurements)
	void printPersonBuffer();
	void getClosestMatch();
	void increaseCount();

};

vector<myMeasurementBuffer> personBuffer;

void 

