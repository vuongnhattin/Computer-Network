#include "Image.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <chrono>
#include "main.h"

using namespace std::chrono;

void initHeaderScreenshot(HeaderScreenshot& header) {
    header.screenDC = GetDC(NULL);
    header.memDC = CreateCompatibleDC(header.screenDC);
    header.bitmap = CreateCompatibleBitmap(header.screenDC, screenWidth, screenHeight);
    SelectObject(header.memDC, header.bitmap);

    header.bi.biSize = sizeof(BITMAPINFOHEADER);
    header.bi.biWidth = screenWidth;
    header.bi.biHeight = -screenHeight;
    header.bi.biPlanes = 1;
    header.bi.biBitCount = 8 * DEPTH;
    header.bi.biCompression = 0;
    header.bi.biSizeImage = 0;
    header.bi.biXPelsPerMeter = 0;
    header.bi.biYPelsPerMeter = 0;
    header.bi.biClrUsed = 0;
    header.bi.biClrImportant = 0;
    BitBlt(header.memDC, 0, 0, screenWidth, screenHeight, header.screenDC, 0, 0, SRCCOPY);
}

cv::Mat capture(HeaderScreenshot& header) {
    cv::Mat frame(screenHeight, screenWidth, CV_8UC4);
    BitBlt(header.memDC, 0, 0, screenWidth, screenHeight, header.screenDC, 0, 0, SRCCOPY);
    GetDIBits(header.memDC, header.bitmap, 0, screenHeight, frame.data, (BITMAPINFO*)&header.bi, DIB_RGB_COLORS);

    return frame;
}

std::vector<uchar> compressImage(cv::Mat image, int quality) {
    std::vector<uchar> buffer;
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality);

    cv::imencode(".jpg", image, buffer, compression_params);
    return buffer;
}

void freeHeaderScreenshot(HeaderScreenshot& header) {
    DeleteObject(header.bitmap);
    DeleteDC(header.memDC);
    ReleaseDC(NULL, header.screenDC);
}

void receiveImageACK(SOCKET acceptImageSocket) {
	char ack[3];
	recv(acceptImageSocket, ack, 3, 0);
    if (strcmp(ack, "NO") == 0) {
        state = State::QUIT;
	}
}

void captureAndSendImage(SOCKET acceptImageSocket ,HeaderScreenshot header) {
    while (state != State::QUIT) {
        auto start = high_resolution_clock::now();
        cv::Mat frame = capture(header);
        //cv::resize(frame, frame, cv::Size(), 0.8, 0.8);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        std::cout << "capture() took: " << duration.count() << "ms\n";
        
        start = high_resolution_clock::now();

        std::vector<uchar> compressed = compressImage(frame, 80);

        stop = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(stop - start);
        std::cout << "compress() took: " << duration.count() << "ms\n";

        int size = compressed.size();
        int byteSent = send(acceptImageSocket, (char*)&size, sizeof(int), 0);
        if (byteSent < 0) {
            std::cout << "Error at send()\n";
            WSACleanup();
            exit(-1);
        }

        std::cout << "Sent: " << byteSent << " bytes\n";
        byteSent = send(acceptImageSocket, (char*)&compressed[0], compressed.size(), 0);
        if (byteSent < 0) {
            std::cout << "Could not send compressed.data()!\n";
            WSACleanup();
            exit(-1);
        }
        std::cout << "Sent: " << byteSent << " bytes\n";

        receiveImageACK(acceptImageSocket);
    }
}