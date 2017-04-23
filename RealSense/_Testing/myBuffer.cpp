#include <stdio.h>
#include <stdlib.h>
#include "myBuffer.h"

int main() {
	printf("testing buffer...\n");

	myBuffer buff;

	location loc1 = createLocation(1,2,4.0);
	printLocation(buff.getLastLocation());
	printf("\n");


	buff.add(loc1);
	buff.printBuffer();
	printLocation(buff.getLastLocation());
	printf("\n");


	location loc2 = createLocation(4,5,6.0);
	buff.add(loc2);
	buff.printBuffer();
	printLocation(buff.getLastLocation());
	printf("\n");


	location loc3 = createLocation(7,8,10.0);
	buff.add(loc3);
	buff.printBuffer();
	printLocation(buff.getLastLocation());
	printf("\n");



	location loc4 = createLocation(12,13,15.0);
	buff.add(loc4);
	buff.printBuffer();
	printLocation(buff.getLastLocation());



}