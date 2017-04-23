/* Need this in here if windows.h is used in program, which is the case in my RealSense applications */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h> /* For _getch */
#include <string.h>
#include <iostream>
#include <sstream> /* For getline */

using namespace std;


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
//#define DEFAULT_ADDR "10.1.10.69"

#define DEFAULT_ADDR "127.0.0.1"

void printBuffer(char[], int);


}

int main()
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char *sendbuf = "this is a test 1234";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;



	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		/* setting timeout for receiving over TCPIP */
		timeval tv;
		tv.tv_sec = 2; /* 2 sec timeout */
		setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval));

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		else { //successfully connected to server
			printf("Connected to server %s\n", DEFAULT_ADDR);
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);


	printf("Waiting for initial response...\n");
	//Receives initial response
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);


	if (iResult <0 ) {
		printf("Mutha...fuckin...error...\n");
	}
	else {
		printf("Bytes received: %d\n", iResult);
		printBuffer(recvbuf, iResult);
	}



	// shutdown the connection since no more data will be sent
	/*iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}*/

	string inputString = "";


	// Send until client terminates the connection
	int count = 0;
	do {
		printf("Enter a string to send to the server: \n");
		getline(cin, inputString);
		const char* toSend = inputString.c_str();
		/* iResult is the number of bytes sent */
		iResult = send(ConnectSocket, toSend, (int)strlen(toSend), 0);
		printf("Bytes sent: %d\n", iResult);
		
		if (iResult > 0) {
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult < 0) {
				printf("error :(\n");
			}
			else {
				printf("Bytes received: %d\n", iResult);
				printBuffer(recvbuf, iResult);
			}

		}
		/* 0 bytes received, so end the loop */
		/*else if (iResult == 0)
			printf("Connection closed\n");*/
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
		count++;


	} while (/*iResult > 0 && */count < 5);

	// cleanup and close socket
	printf("Press q to quit...\n");
	while (true) {
		if (_kbhit()) { // Break loop
			int c = _getch() & 255;
			if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
		}
	}
	closesocket(ConnectSocket);
	WSACleanup();
	printf("Closing program...");
	return 0;
}

void printBuffer(char bufPtr[], int numBytes) {
	printf("Printing buffer...\n");
	for (int i = 0; i < numBytes; i++) {
		printf("%c", bufPtr[i]);
	}
	printf("\n");
}


makeRandomString() {


}
