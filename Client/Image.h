#pragma once
#include <WinSock2.h>
#include <SDL.h>
#include <opencv2/opencv.hpp>

cv::Mat decompressImage(std::vector<char> buffer);
void sendImageACK();
cv::Mat receiveImage();