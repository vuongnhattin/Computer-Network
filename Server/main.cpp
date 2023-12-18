#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <vector>
#include "Image.h"
#include "Socket.h"
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <thread>
#include "Keyboard.h"
#include "Mouse.h"
#include "UI.h"
#include "main.h"

#pragma comment(lib, "ws2_32.lib")

const int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

char ip[16] = "";

SOCKET imageSocket, acceptImageSocket;
SOCKET mouseSocket, acceptMouseSocket;
SOCKET keyboardSocket, acceptKeyboardSocket;

HeaderScreenshot header;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Rect screenRect;
SDL_Event event;

UIState uiState = UIState::DISPLAY_CONNECTION_MENU;
BindingState bindingState = BindingState::BINDING;
ConnectionState connectionState = ConnectionState::NOT_YET;
DiscoveryState discoveryState = DiscoveryState::NOT_YET;

int main(int argc, char** agrv) {
    initUI();
    initHeaderScreenshot(header);

    while (uiState != UIState::QUIT) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                uiState = UIState::QUIT;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        switch (uiState) {
        case UIState::DISPLAY_CONNECTION_MENU:
            displayConnectMenu();
            break;
            
        case UIState::START_THREADS:
            uiState = UIState::DISPLAY_CONTROL_PANEL;
            sendScreenResolution(acceptImageSocket, screenWidth, screenHeight);

            initServerSocket(mouseSocket, acceptMouseSocket, ip, mousePort);
            initServerSocket(keyboardSocket, acceptKeyboardSocket, ip, keyboardPort);

            {
                std::thread keyboardThread(handleKeyboard, acceptKeyboardSocket);
                keyboardThread.detach();

                std::thread mouseThread(handleMouse, acceptMouseSocket);
                mouseThread.detach();

                std::thread sendImageThread(captureAndSendImage, acceptImageSocket, header);
                sendImageThread.detach();
            }
            break;

        case UIState::DISPLAY_CONTROL_PANEL:
            displayControlPanel();
            break;
        case UIState::STOP:
            std::this_thread::sleep_for(std::chrono::seconds(2));
            freeHeaderScreenshot(header);
            WSACleanup();
            closesocket(imageSocket); closesocket(acceptImageSocket);
            closesocket(mouseSocket); closesocket(acceptMouseSocket);
            closesocket(keyboardSocket); closesocket(acceptKeyboardSocket);
            freeUI();
            SDL_Quit();
            uiState = UIState::DISPLAY_CONNECTION_MENU;
            connectionState = ConnectionState::NOT_YET;
            bindingState = BindingState::BINDING;
            discoveryState = DiscoveryState::SUCCESS;
            initUI();
            initHeaderScreenshot(header);
        }
    }

    freeUI();

    freeHeaderScreenshot(header);
    WSACleanup();
    closesocket(imageSocket); closesocket(acceptImageSocket);
    closesocket(mouseSocket); closesocket(acceptMouseSocket);
    closesocket(keyboardSocket); closesocket(acceptKeyboardSocket);

    return 0;
}

