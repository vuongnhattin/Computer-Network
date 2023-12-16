#include "Image.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <SDL.h>
#include <WinSock2.h>
#include <chrono>
#include <thread>
#include "main.h"

cv::Mat decompressImage(std::vector<char> buffer) {
	cv::Mat decompressedImage = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);
	return decompressedImage;
}

void sendImageACK() {
	if (uiState == UIState::QUIT) send(imageSocket, "NO", 2, 0);
	else send(imageSocket, "OK", 2, 0);
}

cv::Mat receiveImage() {
	int size;
	int byteReceived = recv(imageSocket, (char*)&size, sizeof(int), 0);
    if (byteReceived < 0) {
		std::cout << "Error at recv()!\n";
		return cv::Mat();
	}
	
	std::vector<char> buffer(size);
	int totalByteReceived = 0;
    while (totalByteReceived < size) {
		byteReceived = recv(imageSocket, reinterpret_cast<char*>(&buffer[0]) + totalByteReceived, size - totalByteReceived, 0);
        if (byteReceived < 0) {
			std::cout << "Error at recv()!\n";
			return cv::Mat();
		}
		totalByteReceived += byteReceived;
	}

	sendImageACK();

	return decompressImage(buffer);
}

