#include <stdlib.h>
#include <Windows.h>
#include <Tchar.h>
#include <conio.h>
#include <stdio.h>
#include <vector>

using namespace std;


HANDLE wHnd;    // Handle to write to the console.
HANDLE rHnd;    // Handle to read from the console.

int consoleWidth = 60;
int consoleHeight = 20;

void createOutputTable();
void printTable(bool, double);

vector<double> personsFound;

int main() {

	createOutputTable();

	personsFound.push_back(1.2);
	personsFound.push_back(2.3);
	personsFound.push_back(3.4);

	//printTable(false, 0.0, 0, personsFound);
	printTable(true, 0.0);

	Sleep(2000); //in milliseconds
	//printTable(true, 0.0, 0, personsFound);
	//Sleep(2000);
	//printTable(true, 0.0, 1, personsFound);
	//Sleep(2000);
	//printTable(true, 0.0, 2, personsFound);
	//Sleep(2000);
	//printTable(true, 0.0, 3, personsFound);

	while (true) {
		if (_kbhit()) { // Break loop
			int c = _getch() & 255;
			if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
		}
	}

}

void createOutputTable() {
	/* Set up the handles for reading/writing */
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);

	/* Change the window title */
	SetConsoleTitle(TEXT("userID"));

	/* Set up the required window size */
	SMALL_RECT windowSize = { 0, 0, consoleHeight - 1, consoleWidth - 1 };

	/* Change the console window size */
	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

	/* Create a COORD to hold the buffer size */
	COORD bufferSize = { consoleHeight, consoleWidth };

	/* Change the internal buffer size */
	SetConsoleScreenBufferSize(wHnd, bufferSize);

	// Set up the character:
	CHAR_INFO border;
	border.Char.AsciiChar = '*';
	border.Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;

	// Set up the positions:

	for (int i = 0; i < consoleWidth; i++) {  //makes top border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { i,0,i,0 }; //top left coordinates, bottom right coordinates of drawing area
		// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

	for (int i = 0; i < consoleWidth; i++) { //makes bottom border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { i,consoleHeight,i,consoleHeight }; //top left coordinates, bottom right coordinates of drawing area
											// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

	for (int i = 0; i < consoleHeight; i++) { //makes left border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { 0,i,0,i }; //top left coordinates, bottom right coordinates of drawing area
																	// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

	for (int i = 0; i <= consoleHeight; i++) { //makes right border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { consoleWidth,i,consoleWidth,i }; //top left coordinates, bottom right coordinates of drawing area
											// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

}


void printTable(bool targetFound, double targetUserVal) {

	//need to set cursor position before writing to screen

	if (targetFound == false) {
		COORD cursorPos = {2, 1};
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("TargetUser:    N/A");
	}
	else {
		COORD cursorPos = { 2, 1 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("TargetUser:    %.1f", targetUserVal);
	}

	for (int i = 0; i < personsFound.size(); i++) {
		COORD cursorPos = { 2, i + 2 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("person %d:     %.1f", i, personsFound.at(i));

	}

}


/*
// Set up the character:
CHAR_INFO letterA;
letterA.Char.AsciiChar = 'A';
letterA.Attributes =
FOREGROUND_RED | FOREGROUND_INTENSITY |
BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;

// Set up the positions:
COORD charBufSize = { 1,1 };
COORD characterPos = { 0,0 };
SMALL_RECT writeArea = { 0,0,0,0 };

// Write the character:
WriteConsoleOutputA(wHnd, &letterA, charBufSize, characterPos, &writeArea);*/