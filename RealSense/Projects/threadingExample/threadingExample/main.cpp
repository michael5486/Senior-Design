#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <vector>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>

#define MAX_THREADS 1
#define BUF_SIZE 255

using namespace std;

DWORD WINAPI updateTable(LPVOID);
void ErrorHandler(LPTSTR lpszFunction);

// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
/*typedef struct MyData {
	int val1;
	int val2;
} MYDATA, *PMYDATA;*/

HANDLE wHnd;    // Handle to write to the console.
HANDLE rHnd;    // Handle to read from the console.

int consoleWidth = 60;
int consoleHeight = 20;

void createOutputTable();
void printTable(bool, double);

vector<double> personsFound;  //volatile because it will be accessed by multiple threads, volatile makes it so the compiler will not cache anything



int main()
{

	srand(0); //seeds the random generator


	//PMYDATA pDataArray[MAX_THREADS];
	DWORD   dwThreadIdArray[MAX_THREADS];
	HANDLE  hThreadArray[MAX_THREADS];


	/* Populate personsFound */
	personsFound.push_back(1.2);
	personsFound.push_back(2.3);
	personsFound.push_back(3.4);
	createOutputTable();


	// Create MAX_THREADS worker threads.
	for (int i = 0; i<MAX_THREADS; i++) {
	

		// Create the thread to begin execution on its own.

		hThreadArray[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			updateTable,       // thread function name
			NULL,          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadIdArray[i]);   // returns the thread identifier 


									// Check the return value for success.
									// If CreateThread fails, terminate execution. 
									// This will automatically clean up threads and memory. 

		if (hThreadArray[i] == NULL)
		{
			ErrorHandler(TEXT("CreateThread"));
			ExitProcess(3);
		}
	} // End of main thread creation loop.


	for (int i = 0; i < 8; i++) {

		personsFound.push_back(i);
		Sleep(1000);

	}

	for (int i = 0; i < 5000; i++) {
		Sleep(500);
		double person = rand() % 7;
		double value = rand() % 100;
		personsFound[person] = value;
	}


	  /* Wait until all threads have terminated. */
	//WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

	/* Close all thread handles and free memory allocations. */
	for (int i = 0; i<MAX_THREADS; i++) {
		CloseHandle(hThreadArray[i]);
	}

	while (true) {
		if (_kbhit()) { // Break loop
			int c = _getch() & 255;
			if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
		}
	}

	return 0;
}

/* Parameter is void, can be any data type or no data at all. This function is executed by the thread */
DWORD WINAPI updateTable(LPVOID lpParam)
{
	
	for (int i = 0; i < 10000; i++) {


		Sleep(500); //runs every 2 seconds
		printTable(true, 0.0); //prints all the persons in the table

	}


	return 0;
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
		COORD cursorPos = { 2, 1 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("TargetUser:    N/A");
	}
	else {
		COORD cursorPos = { 2, 1 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("TargetUser:    %.1f", targetUserVal);
	}

	for (int personCount = 0; personCount < personsFound.size(); personCount++) {
		COORD cursorPos = { 2, personCount + 3 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("person %d:     %.1f", personCount, personsFound.at(personCount));
	}
}

void ErrorHandler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
