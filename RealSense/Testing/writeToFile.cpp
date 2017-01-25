#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>

using namespace std;


int main() {

	/* Iinitialize random seed. Needs to happen in a method */
	srand(0);


	char separator = ' ';
	int countWidth = 15;
	int headWidth = 25;
	int shoulderWidth = 25;
	int handWidth = 25;
	int spineWidth = 25;



	ofstream outputFile;
	outputFile.open("example.txt");
	outputFile << "Hello world!!!\n";

	for (int i = 0; i < 10; i++) {
		outputFile << i << ", ";
	}

	outputFile << "\n";

	int timeCounter = 0;

	outputFile << left << setw(countWidth) << setfill(separator) << "Time";
	outputFile << left << setw(headWidth) << setfill(separator) << "JOINT_HEAD";
	outputFile << left << setw(shoulderWidth) << setfill(separator) << "JOINT_SHOULDER_LEFT";
	outputFile << left << setw(shoulderWidth) << setfill(separator) << "JOINT_SHOULDER_RIGHT";
	outputFile << left << setw(handWidth) << setfill(separator) << "JOINT_HAND_LEFT";
	outputFile << left << setw(handWidth) << setfill(separator) << "JOINT_HAND_RIGHT";
	outputFile << left << setw(spineWidth) << setfill(separator) << "JOINT_SPINE_MID";
	outputFile << "\n";

	while (timeCounter < 100) {
		outputFile << left << setw(countWidth) << setfill(separator) << timeCounter++;
		outputFile << left << setw(headWidth) << setfill(separator) << rand() % 100;;
		outputFile << left << setw(shoulderWidth) << setfill(separator) << rand() % 100;
		outputFile << left << setw(shoulderWidth) << setfill(separator) << rand() % 100;
		outputFile << left << setw(handWidth) << setfill(separator) << rand() % 100;
		outputFile << left << setw(handWidth) << setfill(separator) << rand() % 100;
		outputFile << left << setw(spineWidth) << setfill(separator) << rand() % 100;
		outputFile << "\n";

	}

	/*outputFile << left << setw(nameWidth) << setfill(separator) << "Michael";
	outputFile << left << setw(nameWidth) << setfill(separator) << "Esposito";
	outputFile << "\n";
	outputFile << left << setw(nameWidth) << setfill(separator) << "Sean";
	outputFile << left << setw(nameWidth) << setfill(separator) << "Bird";*/






	outputFile.close();


}