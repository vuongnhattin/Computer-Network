#pragma once
#include <opencv2/opencv.hpp>

enum class State {
	CONNECT_MENU,
	INIT_CONTENT,
	DISPLAY_IMAGE,
	QUIT,
};

enum class ConnectionState {
	NOT_YET,
	FAIL,
	SUCCESS,
};

void initUI();
void freeUI();
void displayConnectPanel();
void renderControlPanel();
void renderImage(cv::Mat image);
void receiveAndDisplayImage();