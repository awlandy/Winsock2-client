#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <iostream>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

constexpr char OPTION_VALUE = 1;
constexpr auto SERVER_ADDRESS = "127.0.0.1";
constexpr auto DEFAULT_BUFLEN = 512;
constexpr auto DEFAULT_PORT = "27015";


void recieving(SOCKET ConnectSocket);

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult = 0;

	// Initialize Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	getaddrinfo(SERVER_ADDRESS, DEFAULT_PORT, &hints, &result);

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

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	


	std::string test = "this is a test";
	std::string sendbuf = "";
	send(ConnectSocket, test.c_str(), strlen(test.c_str()), 0);

	// Receive until the peer closes the connection
	std::thread t1(recieving, std::ref(ConnectSocket));

	// Input and send loop
	while(1) {
		std::getline(std::cin, sendbuf);
		send(ConnectSocket, sendbuf.c_str(), strlen(sendbuf.c_str()), 0);
		printf("Client :%s\n", sendbuf.c_str());
	};


	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

void recieving(SOCKET ConnectSocket) {
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	do {
		memset(recvbuf, 0, DEFAULT_BUFLEN * sizeof(char));
		if (ConnectSocket != 0) {
			iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0)
				printf("Sever: %s\n", recvbuf);
			else if (iResult == 0)
				printf("Connection closed\n");
			else
				printf("recv failed with error: %d\n", WSAGetLastError());

		}
	} while (iResult > 0);
}