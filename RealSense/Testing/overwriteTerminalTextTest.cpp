/* Testing how I can overwrite old text in the terminal */

/* Want output to look like this:

        userID    -   Michael Esposito
*************************************************
*                                               *
*    Target User: N/A                           * 
*                                               *
*    Person1:  N/A                              *
*                                               *
*************************************************

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void printTable(bool, double, int, double[]);


int main() {

	double outputValues[] = {1.2, 2.3, 3.4};

	printTable(false, 0.0, 0, outputValues);
	sleep(2);
	printTable(true, 0.0, 0, outputValues);
	sleep(2);
	printTable(true, 2.3, 1, outputValues);
	sleep(2);
	printTable(true, 2.3, 2, outputValues);
	sleep(2);
	printTable(true, 2.3, 3, outputValues);


}


void printTable(bool targetFound, double targetUserVal, int numUsersFound, double outputValues[]) {
	
	/* Clears old text from the terminal */
	system("clear");
	
	printf("        userID    -   Michael Esposito\n");
	printf("*************************************************\n");
	printf("*                                               *\n");

	if (targetFound == false) {
		printf("*    Target User: N/A                           *\n"); 
	}
	else {
		printf("*    Target User: %.1f                           *\n", targetUserVal);
	}

	printf("*                                               *\n");


	for (int i = 0; i < numUsersFound; i++) {
		printf("*    Person1:  %.1f                              *\n", outputValues[i - 1]);
	}
	printf("*                                               *\n");
	printf("*************************************************\n");



}