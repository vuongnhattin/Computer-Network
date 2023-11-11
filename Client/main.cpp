#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <thread>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <SDL.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std::chrono;
using namespace std;

#define PORT 55555

const int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

void initSocket(SOCKET& s) {
	WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		printf("WSAStartup fail!\n");
		return;
	}
    else {
		printf("WSAStartup ok!\n");
	}

	s = INVALID_SOCKET;
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
		printf("Error at socket()!\n");
		WSACleanup();
		return;
	}
    else {
		printf("socket() is ok!\n");
	}

	char ipv4[80];
	cout << "Nhap dia chi IP: ";
	cin >> ipv4;

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, ipv4, &servAddr.sin_addr.s_addr);
	servAddr.sin_port = htons((u_short)PORT);
	int servAddrLen = (int)sizeof(servAddr);

    // connect
    if (connect(s, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        cout << "connect() failed\n";
        WSACleanup();
        exit(-1);
    }
    else {
        cout << "connect() is ok!\n";
    }
}

int main(int argc, char** argv) {
    SOCKET s;
    initSocket(s);
    
    SDL_Init(SDL_INIT_VIDEO);
    const double scale = 0.8;
    SDL_Window* window = SDL_CreateWindow("SDL2 Displaying Image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth * scale, screenHeight * scale, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
				quit = true;
			}
		}
        auto start = high_resolution_clock::now();
        int size;
        int byteReceived = recv(s, (char*)&size, sizeof(int), 0);
        if (byteReceived < 0) {
			cout << "Error at recv()!\n";
			exit(-1);
		}
        else {
			cout << "Received size: " << size << endl;
		}
        vector<char> buffer(size);
        int totalByteReceived = 0;
        while (totalByteReceived < size) {
            byteReceived = recv(s, reinterpret_cast<char*>(&buffer[0]) + totalByteReceived, size - totalByteReceived, 0);
            if (byteReceived < 0) {
				cout << "Error at recv()!\n";
				exit(-1);
			}
			totalByteReceived += byteReceived;
        }

        auto start2 = high_resolution_clock::now();
        cv::Mat decompressedImage = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(decompressedImage.data, decompressedImage.cols, decompressedImage.rows, decompressedImage.channels() * 8, decompressedImage.step, 0xff0000, 0x00ff00, 0x0000ff, 0);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        auto stop2 = high_resolution_clock::now();
        auto duration2 = duration_cast<milliseconds>(stop2 - start2);
        cout << "Decompress time: " << duration2.count() << " ms\n";

        cout << "DecompressedImage size: " << decompressedImage.size() << endl;
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        /*cv::imshow("Display frame", decompressedImage);

        if (cv::waitKey(1) == 27) {
            break;
        }*/

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "FPS: " << (double)1 / duration.count() * 1000000 << endl;
    }

    closesocket(s);
    WSACleanup();

    return 0;
}