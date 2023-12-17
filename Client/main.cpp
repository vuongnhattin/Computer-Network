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

UIState uiState = UIState::DISPLAY_CONNECT_MENU;
ConnectionState connectState = ConnectionState::NOT_YET;
DiscoverState discoverState = DiscoverState::NOT_YET;

int main(int argc, char** argv) {
    initUI();

    while (uiState != UIState::QUIT) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                uiState = UIState::QUIT;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        switch (uiState) {
        case UIState::DISPLAY_CONNECT_MENU:
            displayConnectMenu();
            break;

        case UIState::START_THREADS:
            getServerScreenResolution();

            initClientSocket(mouseSocket, ip, mousePort);
            initClientSocket(keyboardSocket, ip, keyboardPort);

            {
                std::thread keyboardThread(sendKeyboardEvents);
                keyboardThread.detach();

                std::thread mouseThread(sendMouseEvents);
                mouseThread.detach();

                std::thread imageThread(receiveAndDisplayImage);
                imageThread.detach();
            }

            uiState = UIState::DISPLAY_IMAGE;
            break;

        case UIState::DISPLAY_IMAGE: break;

        case UIState::STOP:
            std::this_thread::sleep_for(std::chrono::seconds(2));
            WSACleanup();
            closesocket(imageSocket);
            closesocket(mouseSocket);
            closesocket(keyboardSocket);
            freeUI();
            SDL_Quit();
            std::cout << "Disconected.\n";
            uiState = UIState::DISPLAY_CONNECT_MENU;
            connectState = ConnectionState::NOT_YET;
            discoverState = DiscoverState::NOT_YET;
            initUI();
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

