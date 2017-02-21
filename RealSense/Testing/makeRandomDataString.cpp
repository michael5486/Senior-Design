#include <string.h>
#include <iostream>
#include <sstream> /* For getline */

#include <stdlib.h>
#include <stdio.h>
#include <random>

using namespace std;


string makeRandomString();



int main() {

	srand(0);
	printf("random string: %s\n", makeRandomString().c_str());

}

string makeRandomString() {
	stringstream ss;

	ss << left << rand() % 340;
	ss << left << " ";
	ss << left << rand() % 240;

	return ss.str();
}