#pragma once
#include <WinSock2.h>

enum class State {
	DISPLAY_CONNECTION_MENU,
	START_THREADS,
	DISPLAY_CONTROL_PANEL,
	QUIT,
};

enum class BindingState {
	BINDING,
	FAILED,
	BOUND,
};

enum class ConnectionState {
	NOT_YET,
	WAITING,
	CONNECTED,
};

void initUI();
void freeUI();
void displayConnectMenu();
void waitingForConnection(SOCKET& serverSocket, SOCKET& acceptSocket);
void displayControlPanel();