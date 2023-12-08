#pragma once
#include <WinSock2.h>

bool initSocket(SOCKET& s, SOCKET& acceptSocket, const char* ip, const int port);
bool listenSocket(SOCKET& serverSocket, SOCKET& acceptSocket);
bool bindSocket(SOCKET& serverSocket, const char* ip, const int port);