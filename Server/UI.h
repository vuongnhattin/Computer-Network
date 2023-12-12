#pragma once
#include <WinSock2.h>

enum class State {
	CONNECT_MENU,
	SENDING_CONTENT,
};

enum class ConnectionState {
	BINDING,
	BIND_FAILED,
	BOUND,
	CONNECTING,
	CONNECTED,
};

void initUI();
void freeUI();
void displayConnectPanel();
