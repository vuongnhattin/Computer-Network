#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <SDL.h>
#include "Socket.h"
#include "Image.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Mouse.h"
#include "main.h"
#include "UI.h"
#include "Keyboard.h"
#pragma comment(lib, "ws2_32.lib")

int screenWidth = GetSystemMetrics(SM_CXSCREEN), screenHeight = GetSystemMetrics(SM_CYSCREEN);
int serverScreenWidth = 1280, serverScreenHeight = 720;

char ip[16] = "";

SDL_Event event;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Rect screenRect;

SOCKET imageSocket, mouseSocket, keyboardSocket;

State state = State::CONNECT_MENU;
ConnectionState connectState = ConnectionState::NOT_YET;

int main(int argc, char** argv) {
    initUI();

    while (state != State::QUIT) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                state = State::QUIT;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        switch (state) {
        case State::CONNECT_MENU:
            displayConnectPanel();
            break;

        case State::INIT_CONTENT:
            getServerScreenResolution();

            initClientSocket(mouseSocket, ip, mousePort);
            initClientSocket(keyboardSocket, ip, keyboardPort);

            {
                std::thread keyboardThread(sendKeyboardEvents);
                keyboardThread.detach();

                std::thread mouseThread(sendMouseEvents);
                mouseThread.detach();

                std::thread imageThread(displayImage);
                imageThread.detach();
            }

            state = State::DISPLAY_IMAGE;
            break;

        case State::DISPLAY_IMAGE: break;

        default:
            break;
        }
    }

    freeUI();

    WSACleanup();
    closesocket(imageSocket);
    closesocket(mouseSocket);
    closesocket(keyboardSocket);
    
    return 0;
}

