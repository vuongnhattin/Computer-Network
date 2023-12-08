#include "Image.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <SDL.h>
#include <WinSock2.h>
#include <chrono>
#include <thread>
#include "main.h"

using namespace std::chrono;

cv::Mat receiveImage(SOCKET clientSocket) {
	auto start = high_resolution_clock::now();
	int size;
	int byteReceived = recv(clientSocket, (char*)&size, sizeof(int), 0);
    if (byteReceived < 0) {
		std::cout << "Error at recv()!\n";
		return cv::Mat();
	}
    else {
		std::cout << "Received size: " << size << std::endl;
	}
	std::vector<char> buffer(size);
	int totalByteReceived = 0;
    while (totalByteReceived < size) {
		byteReceived = recv(clientSocket, reinterpret_cast<char*>(&buffer[0]) + totalByteReceived, size - totalByteReceived, 0);
        if (byteReceived < 0) {
			std::cout << "Error at recv()!\n";
			return cv::Mat();
		}
		totalByteReceived += byteReceived;
	}

	if (quit) send(clientSocket, "NO", 2, 0);
	else send(clientSocket, "OK", 2, 0);

	auto start2 = high_resolution_clock::now();

	cv::Mat decompressedImage = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);

	auto stop2 = high_resolution_clock::now();
	auto duration2 = duration_cast<milliseconds>(stop2 - start2);
	std::cout << "Decompress time: " << duration2.count() << " ms\n";

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	std::cout << "FPS: " << (double)1 / duration.count() * 1000000 << std::endl;

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