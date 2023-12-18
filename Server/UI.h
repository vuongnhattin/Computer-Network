#pragma once
#include <WinSock2.h>

enum class UIState {
	DISPLAY_CONNECTION_MENU,
	START_THREADS,
	DISPLAY_CONTROL_PANEL,
	QUIT,
	STOP,
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

enum class DiscoveryState {
	NOT_YET,
	DISCOVERING,
	SUCCESS,
	NONE,
};

void initUI();
void freeUI();
void displayConnectMenu();
void waitingForConnection(SOCKET& serverSocket, SOCKET& acceptSocket);
void displayControlPanel();