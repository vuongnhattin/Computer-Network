#pragma once
#include <WinSock2.h>

bool initSocket(SOCKET& s, char* ip, const int port);