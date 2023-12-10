#include "Image.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <SDL.h>
#include <WinSock2.h>
#include <chrono>
#include <thread>
#include "main.h"

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

	if (state == State::QUIT) send(imageSocket, "NO", 2, 0);
	else send(imageSocket, "OK", 2, 0);

	cv::Mat decompressedImage = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);

	return decompressedImage;
}

void renderImage(cv::Mat image) {
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image.data, image.cols, image.rows, image.channels() * 8, image.step, 0xff0000, 0x00ff00, 0x0000ff, 0);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, NULL, &screenRect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	std::this_thread::sleep_for(std::chrono::milliseconds(0));
}