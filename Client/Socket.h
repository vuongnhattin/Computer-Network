#pragma once
#include <WinSock2.h>

bool initClientSocket(SOCKET& s, char* ip, const int port);
void getServerScreenResolution();