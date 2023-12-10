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
#include "main.h"
#include "Keyboard.h"
#include "Mouse.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std::chrono;

const int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
const int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

bool quit = false, validIP = true, bound = false, connected = false, buttonEnabled = true, isWaitingClient = false, startedMouseKeyboadSocket = false, startedSendImage = false, sentScreenResolution = false;
char ip[16] = "";
SOCKET imageSocket, acceptImageSocket;
SOCKET mouseSocket, acceptMouseSocket;
SOCKET keyboardSocket, acceptKeyboardSocket;
const int imagePort = 55555, mousePort = 55556, keyboardPort = 55557;

int main(int argc, char** agrv) {
    double scale = 0.8;
    int appWidth = screenWidth * scale, appHeight = screenHeight * scale;

    HeaderScreenshot header;
    initHeaderScreenshot(header);
    char buff[3];

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, appWidth, appHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("OpenSans-Regular.ttf", 24);
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        if (!connected) {
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();

            ImGui::SetNextWindowSize(ImVec2(appWidth, appHeight));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            if (ImGui::Begin("Initialize server", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
                ImGui::Text("Server's IP Address");
                ImGui::InputText("##IP", ip, IM_ARRAYSIZE(ip));
                if (ImGui::Button("Initialize")) {
                    if (bindSocket(imageSocket, ip, imagePort)) {
                        bound = true, validIP = true, isWaitingClient = true;
                    }
                    else {
                        validIP = false;
                    }
                }
                if (!validIP) {
                    ImGui::Text("Initialize failed!");
                }
                if (bound) {
                    ImGui::Text("Initialize success!");
                    ImGui::Text("Waiting for client...");

                    if (isWaitingClient) {
                        isWaitingClient = false;
                        auto fun = [](SOCKET& imageSocket, SOCKET& acceptImageSocket) {
                            listenSocket(imageSocket, acceptImageSocket);
                            connected = true;
                            };
                        std::thread t(fun, std::ref(imageSocket), std::ref(acceptImageSocket));
                        t.detach();
                    }
                }
            }
            ImGui::End();

            ImGui::Render();

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        }

        if (connected) {
            if (!sentScreenResolution) {
				sentScreenResolution = true;
				sendScreenResolution(acceptImageSocket, screenWidth, screenHeight);
			}
            if (!startedMouseKeyboadSocket) {
                startedMouseKeyboadSocket = true;
                initServerSocket(mouseSocket, acceptMouseSocket, ip, mousePort);
                initServerSocket(keyboardSocket, acceptKeyboardSocket, ip, keyboardPort);

                std::thread keyboardThread(handleKeyboard, acceptKeyboardSocket);
                keyboardThread.detach();
                std::thread mouseThread(handleMouse, acceptMouseSocket);
                mouseThread.detach();
            }
            
            if (!startedSendImage) {
                startedSendImage = true;
                std::thread sendImageThread(captureAndSendImage, acceptImageSocket, header);
                sendImageThread.detach();
            }

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();

            //ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetWindowSize(ImVec2(200, 100));
            ImGui::SetNextWindowSize(ImVec2(200, 100));
            if (ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_NoResize)) {
                if (ImGui::Button("Exit")) {
                    quit = true;
                }
            }
            ImGui::End();

            ImGui::Render();

            SDL_RenderClear(renderer);

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        }
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    freeHeaderScreenshot(header);
    WSACleanup();
    closesocket(imageSocket); closesocket(acceptImageSocket);
    closesocket(mouseSocket); closesocket(acceptMouseSocket);
    closesocket(keyboardSocket); closesocket(acceptKeyboardSocket);

    return 0;
}

