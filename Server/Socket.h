#pragma once
#include <WinSock2.h>

bool initServerSocket(SOCKET& sererSocket, SOCKET& acceptSocket, const char* ip, const int port);
bool bindSocket(SOCKET& serverSocket, const char* ip, const int port);
bool listenSocket(SOCKET& serverSocket, SOCKET& acceptSocket);
void broadcastS();
void sendScreenResolution(SOCKET acceptSocket, int width, int height);