#include "Image.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <SDL.h>
#include <WinSock2.h>
#include <chrono>
#include <thread>
#include "main.h"

bool decompressImage(cv::Mat &img,std::vector<char> buffer) {
	img = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);
	return true;
}

void sendImageACK() {
	if (uiState != UIState::DISPLAY_IMAGE) send(imageSocket, "NO", 2, 0);
	else send(imageSocket, "OK", 2, 0);
}

bool receiveImage(cv::Mat &img) {
	int size;
	int byteReceived = recv(imageSocket, (char*)&size, sizeof(int), 0);
	if (size == -10) {
		for (int i = 0; i < 1000; i++) {
			std::cout << "received disconnect request.\n";
		}
		return false;
	}
    if (byteReceived < 0) {
		std::cout << "Error at recv()!\n";
		return false;
	}
	
	std::vector<char> buffer(size);
	int totalByteReceived = 0;
    while (totalByteReceived < size) {
		byteReceived = recv(imageSocket, reinterpret_cast<char*>(&buffer[0]) + totalByteReceived, size - totalByteReceived, 0);
        if (byteReceived < 0) {
			std::cout << "Error at recv()!\n";
			return false;
		}
		totalByteReceived += byteReceived;
	}

	sendImageACK();

	return decompressImage(img,buffer);
}

