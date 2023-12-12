#pragma once
#include <WinSock2.h>

bool initServerSocket(SOCKET& sererSocket, SOCKET& acceptSocket, const char* ip, const int port);
void sendScreenResolution(SOCKET acceptSocket, int width, int height);
bool listenSocket(SOCKET& serverSocket, SOCKET& acceptSocket);
void waitingForConnection(SOCKET& serverSocket, SOCKET& acceptSocket);
bool bindSocket(SOCKET& serverSocket, const char* ip, const int port);