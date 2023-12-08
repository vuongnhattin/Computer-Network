#pragma once
#include <SDL.h>
#include <thread>
#include <mutex>

extern int screenWidth, screenHeight;
extern bool quit, connected, validIP, connectedToMouse;
extern char ip[16];
extern SDL_Event event;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SOCKET imageSocket, keyboardSocket;
extern SDL_Rect screenRect;
extern std::mutex mtx;
extern const int imagePort, mousePort, keyboardPort;

