#include "Mouse.h"
#include <SDL.h>
#include <WinSock2.h>
#include <iostream>
#include "main.h"

void sendMousePosition() {
	while (!quit) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		x = (int)((double)x / screenRect.w * screenWidth);
		y = (int)((double)y / screenRect.h * screenHeight);

		std::cout << "Mouse position: " << x << " " << y << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}