#pragma once
#include <WinSock2.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

struct HeaderScreenshot {
    HDC screenDC, memDC;
    HBITMAP bitmap;
    BITMAPINFOHEADER bi;
};

void initHeaderScreenshot(HeaderScreenshot& header);
void freeHeaderScreenshot(HeaderScreenshot& header);
cv::Mat capture(HeaderScreenshot& header);
std::vector<uchar> compressImage(cv::Mat image, int quality);
void receiveImageACK(SOCKET acceptImageSocket);
void captureAndSendImage(SOCKET acceptSocket, HeaderScreenshot header);