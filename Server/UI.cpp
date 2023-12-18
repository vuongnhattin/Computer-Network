#include "UI.h"
#include "Image.h"
#include <WinSock2.h>
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Socket.h"
#include "main.h"
#include <WS2tcpip.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib")
void broadcastS() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverHint;
    ZeroMemory(&serverHint, sizeof(serverHint));
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(broadcastPort);
    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
    {
        std::cout << "Can't bind socket! " << WSAGetLastError() << "\n";
        return;
    }
    std::cout << "bind broadcast socket successfully\n";
    sockaddr_in client;
    int clientLength = sizeof(client);
    char buf[1024];
    std::cout << "waiting for name req from client broadcast.\n";
    while (uiState == UIState::DISPLAY_CONNECTION_MENU)
    {
        ZeroMemory(&client, clientLength);
        ZeroMemory(buf, 1024);
        int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            //std::cout << "Error receiving from client " << WSAGetLastError() << "\n";
            continue;
        }
        char clientIp[256];
        ZeroMemory(clientIp, 256);
        inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
        //std::cout << "Message recv from " << clientIp << " : " << buf << "\n";
        char computerName[100]{ 0 };
        gethostname(computerName, 100);
        std::string computerStr(computerName);
        /*if (bindingState != BindingState::BOUND) {
            computerStr += " not bind";
        }*/
        ZeroMemory(buf, 1024);
        sendto(in, computerStr.c_str(), computerStr.size() + 1, 0, (sockaddr*)&client, clientLength);
        sockaddr_in foo;
        if (recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength) != SOCKET_ERROR) {
            if (inet_pton(AF_INET, buf, &foo.sin_addr.s_addr) == 1) {
                std::cout << "IP recv from " << clientIp << " : " << buf << "\n";
                memcpy(ip, buf, 16);
                discoveryState = DiscoveryState::DISCOVERED;
            }
        }
    }
    closesocket(in);
    std::cout << "shut down broadcast socket.\n";
}
void initUI() {
    const double scale = 0.4;
    int appWidth = screenWidth * scale, appHeight = screenHeight * scale;

    screenRect = { 0, 0, appWidth, appHeight };

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, appWidth, appHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("OpenSans-Regular.ttf", 24);
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

void freeUI() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void waitingForConnection(SOCKET& serverSocket, SOCKET& acceptSocket) {
    listenSocket(serverSocket, acceptSocket);
    connectionState = ConnectionState::CONNECTED;
    uiState = UIState::START_THREADS;
}

void displayConnectMenu() {
    if (discoveryState == DiscoveryState::NOT_YET) {
        discoveryState = DiscoveryState::DISCOVERING;
        std::thread broadcast(broadcastS);
        broadcast.detach();
    }
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(screenRect.w, screenRect.h));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (ImGui::Begin("Initialize server", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Waiting for client connect...");
        if (discoveryState == DiscoveryState::DISCOVERED) {
            connectionState = ConnectionState::WAITING;
            bindSocket(imageSocket, ip, imagePort);
            std::thread waitingForConnectionThread(waitingForConnection, std::ref(imageSocket), std::ref(acceptImageSocket));
            waitingForConnectionThread.detach();
            discoveryState = DiscoveryState::DISCOVERING;
            //std::thread broadcast(broadcastS);
            //broadcast.detach();
        }

        ImGui::Separator();
        if (ImGui::Button("Exit")) {
            uiState = UIState::QUIT;
        }
    }

    ImGui::End();

    ImGui::Render();

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}


void displayControlPanel() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    ImGui::SetWindowSize(ImVec2(200, 100));
    ImGui::SetNextWindowSize(ImVec2(200, 100));
    if (ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_NoResize)) {
        if (ImGui::Button("Disconnect")) {
            uiState = UIState::STOP;
        }
    }
    ImGui::End();

    ImGui::Render();

    SDL_RenderClear(renderer);

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}