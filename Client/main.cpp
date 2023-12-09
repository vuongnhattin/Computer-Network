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
#include <mutex>
#include "Keyboard.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std::chrono;

int screenWidth = GetSystemMetrics(SM_CXSCREEN), screenHeight = GetSystemMetrics(SM_CYSCREEN);
bool quit = false, connected = false, validIP = true, startedMouseThread = false, startedContentThread = false, startedKeyboardThread = false, gotScreenResolution = false;
char ip[16] = "";
SDL_Event event;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Rect screenRect;
std::mutex mtx;
SOCKET imageSocket, mouseSocket, keyboardSocket;

const int imagePort = 55555, mousePort = 55556, keyboardPort = 55557;

int main(int argc, char** argv) {
    initUI();

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        if (!connected) {
            renderLoginPanel();
            SDL_RenderPresent(renderer);
        }

        else {
            if (!gotScreenResolution) {
                gotScreenResolution = true;
                getScreenResolution();
            }
            if (!startedMouseThread) {
				startedMouseThread = true;

                initClientSocket(mouseSocket, ip, mousePort);

                initClientSocket(keyboardSocket, ip, keyboardPort);
                std::thread keyboardThread(sendKeyboardEvents);
                keyboardThread.detach();

                std::thread mouseThread(sendMousePosition);
                mouseThread.detach();
			}

            if (!startedContentThread) {
				startedContentThread = true;

                std::thread imageThread(displayContent);
                imageThread.detach();
			}
        }
    }

    cleanUpUI();

    WSACleanup();
    closesocket(imageSocket);

    return 0;
}

