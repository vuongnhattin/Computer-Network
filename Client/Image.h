#pragma once
#include <WinSock2.h>
#include <SDL.h>
#include <opencv2/opencv.hpp>

cv::Mat receiveImage();
void renderImage(cv::Mat image);