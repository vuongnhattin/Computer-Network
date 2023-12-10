#pragma once
#include <WinSock2.h>

bool initServerSocket(SOCKET& s, SOCKET& acceptSocket, const char* ip, const int port);
void sendScreenResolution(SOCKET acceptSocket, int width, int height);
bool listenSocket(SOCKET& imageSocket, SOCKET& acceptSocket);
bool bindSocket(SOCKET& imageSocket, const char* ip, const int port);