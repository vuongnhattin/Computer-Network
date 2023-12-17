#pragma once
#include <WinSock2.h>
#include <SDL.h>
#include <opencv2/opencv.hpp>

bool decompressImage(cv::Mat& img, std::vector<char> buffer);
void sendImageACK();
bool receiveImage(cv::Mat& img);