#include <vector>   //for vector
#include <stdio.h>  //for console output
#include <iostream>  
#include <stdlib.h>  //for srand
#include <algorithm>  //for sorting
#include <cmath> //for floor


#define VECTOR_MAX_SIZE 99

using namespace std;

vector<int> intVector;

void printVector(vector<int>);
int findMedian(vector<int>);




int main() {
	//size return

	srand(1);


	/* Fills vector with random ints between 0 and 100 */
	for (int i = 0; i < VECTOR_MAX_SIZE; i++) {
		intVector.push_back(rand() % 100);
	}

	printVector(intVector);

	printf("median= %d\n", findMedian(intVector));

}

void printVector(vector<int> vect) {
	printf("pos | val\n");
	printf("________\n");
	int i = 0;
	for (vector<int>::iterator it = vect.begin(); it != vect.end(); it++) {
		printf("%d  |  %d\n", i, *it);
		i++;
	}
}

int findMedian(vector<int> vect) {
	sort(vect.begin(), vect.end());
	printVector(vect);

	int size = vect.size();

	if (vect.empty()) { //returns -1 if vect is empty
		return -1;
	}
	else if (size % 2 == 0) { //if vect contains even number of items
		int medianLoc = size / 2;
		int val1 = vect[medianLoc];
		int val2 = vect[medianLoc - 1];
		printf("medianLoc= %d  medianLoc-1 = %d\n", medianLoc, medianLoc-1);
		return (val1 + val2) / 2;

	}
	else { //vect contains odd # of values
		int medianLoc = size / 2;
		medianLoc = floor(medianLoc);
		printf("medianLoc = %d\n", medianLoc);
		return vect[medianLoc];
	}




}