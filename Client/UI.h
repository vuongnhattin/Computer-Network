#pragma once

enum class State {
	CONNECT_MENU,
	INIT_CONTENT,
	DISPLAY_IMAGE,
	QUIT,
};

enum class ConnectState {
	NOT_YET,
	FAIL,
	SUCCESS,
};

void initUI();
void cleanUpUI();
void displayConnectPanel();
void renderControlPanel();
void displayImage();