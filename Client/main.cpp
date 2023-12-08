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
#pragma comment(lib, "ws2_32.lib")

using namespace std::chrono;

const int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

bool quit = false, connected = false, validIP = true, startedMouseThread = false, startedContentThread = false;
char ip[16] = "";
SDL_Event event;
SDL_Window* window;
SDL_Renderer* renderer;
SOCKET clientSocket, mouseSocket;
SDL_Rect screenRect;
std::mutex mtx;

const int imagePort = 55555, mousePort = 55556, keyboardPort = 55557;

int main(int argc, char** argv) {

    SDL_Init(SDL_INIT_VIDEO);

    const double scale = 0.8;
    const int appWidth = screenWidth * scale, appHeight = screenHeight * scale;

    screenRect = { 0, 0, appWidth, appHeight };

    window = SDL_CreateWindow("Client", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, appWidth, appHeight, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("OpenSans-Regular.ttf", 24);
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

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

        if (connected) {
            if (!startedMouseThread) {
				startedMouseThread = true;
                initSocket(mouseSocket, ip, mousePort);
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

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    WSACleanup();
    closesocket(clientSocket);

    return 0;
}