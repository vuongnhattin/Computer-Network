#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

using std::cout; using std::cin; using std::endl; using std::vector;
using namespace std::chrono;

#define PORT 55555
#define DEPTH 4

int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

struct HeaderScreenshot {
    HDC screenDC, memDC;
    HBITMAP bitmap;
    BITMAPINFOHEADER bi;
};

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

void capture(HeaderScreenshot& header, BYTE* pixels) {

    GetDIBits(header.memDC, header.bitmap, 0, screenHeight, pixels, (BITMAPINFO*)&header.bi, DIB_RGB_COLORS);
}

cv::Mat capture(HeaderScreenshot& header) {
    cv::Mat frame(screenHeight, screenWidth, CV_8UC4);
    BitBlt(header.memDC, 0, 0, screenWidth, screenHeight, header.screenDC, 0, 0, SRCCOPY);
    GetDIBits(header.memDC, header.bitmap, 0, screenHeight, frame.data, (BITMAPINFO*)&header.bi, DIB_RGB_COLORS);

    return frame;
}

vector<uchar> compressImage(cv::Mat image, int quality) {
    vector<uchar> buffer;
    vector<int> compression_params;
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

void initSocket(SOCKET& s, SOCKET& acceptSocket) {
    // Initialize
    WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        printf("WSAStartup fail!\n");
        exit(-1);
    }
    else {
        printf("WSAStartup ok!\n");
    }

    // Create socket
    s = INVALID_SOCKET;
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        printf("Error at socket()!\n");
        WSACleanup();
        exit(-1);
    }
    else {
        printf("socket() is ok!\n");
    }

    char ipv4[80];
    printf("Nhap dia chi IP: ");
    cin >> ipv4;

    // Bind
    sockaddr_in service;
    service.sin_family = AF_INET;
    inet_pton(AF_INET, (const char*)ipv4, &service.sin_addr.s_addr);
    service.sin_port = htons((u_short)PORT);

    if (bind(s, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
        cout << "bind() faild!\n";
        closesocket(s);
        WSACleanup();
        exit(-1);
    }
    else {
        printf("bind() is ok!\n");
    }
    printf("IPv4: %s\n", ipv4);

    // Listen
    if (listen(s, 1) == SOCKET_ERROR) {
        cout << "error at listen()\n";
        WSACleanup();
        exit(-1);
    }
    else {
        cout << "listen() ok!\n";
    }

    // Accept
    acceptSocket = accept(s, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        cout << "failed at accept()\n";
        WSACleanup();
        exit(-1);
    }
    else {
        cout << "listening...\n";
    }
}

int main(int argc, char** agrv) {
    SOCKET s, acceptSocket;
    initSocket(s, acceptSocket);

    HeaderScreenshot header;
    initHeaderScreenshot(header);

    while (1) {
        auto start = high_resolution_clock::now();
        cv::Mat frame = capture(header);
        cv::resize(frame, frame, cv::Size(), 0.8, 0.8);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        cout << "capture() took: " << duration.count() << "ms\n";

        start = high_resolution_clock::now();
        vector<uchar> compressed = compressImage(frame, 80);
        stop = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(stop - start);
        cout << "compress() took: " << duration.count() << "ms\n";

        int size = compressed.size();
        int byteSent = send(acceptSocket, (char*)&size, sizeof(int), 0);
        if (byteSent < 0) {
            int error = WSAGetLastError();
            cout << "Could not send size! Error code: " << error << endl;
            WSACleanup();
            break;
        }
        cout << "Sent: " << byteSent << " bytes\n";
        byteSent = send(acceptSocket, (char*)&compressed[0], compressed.size(), 0);
        if (byteSent < 0) {
            cout << "Could not send compressed.data()!\n";
            WSACleanup();
            break;
        }
        cout << "Sent: " << byteSent << " bytes\n";
    }

    freeHeaderScreenshot(header);
    WSACleanup();
    closesocket(s); closesocket(acceptSocket);

    return 0;
}