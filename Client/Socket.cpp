#include "Socket.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "main.h"

bool initClientSocket(SOCKET& clientSocket, char* ip, const int port) {
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		std::cout << "WSAStartup fail!\n";
		return false;
	}
	else {
		std::cout << "WSAStartup ok!\n";
	}

	clientSocket = INVALID_SOCKET;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "Error at socket()!\n";
		WSACleanup();
		return false;
	}
	else {
		std::cout << "socket() is ok!\n";
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip, &servAddr.sin_addr.s_addr) != 1) {
		std::cout << "Invalid address!\n";
		WSACleanup();
		return false;
	}
	servAddr.sin_port = htons((u_short)port);
	int servAddrLen = (int)sizeof(servAddr);

	if (connect(clientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		std::cout << "connect() failed\n";
		WSACleanup();
		return false;
	}
	else {
		std::cout << "connect() is ok!\n";
	}

	return true;
}

void getScreenResolution() {
	char buffer[10];

	recv(imageSocket, buffer, 10, 0);
	screenWidth = atoi(buffer);

	recv(imageSocket, buffer, 10, 0);
	screenHeight = atoi(buffer);
}