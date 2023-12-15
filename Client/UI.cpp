#define _CRT_SECURE_NO_WARNINGS
#include "UI.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_impl_sdl2.h"
#include "Image.h"
#include "main.h"
#include "Socket.h"
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

std::set < std::pair<std::string, std::string>> serversSet;

char** ipList;
char** hostnameList;
char** ipAndHostnameList;
int currentItem = 0;

void broadcastC(char *ip) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sockaddr_in serverHint;
	serverHint.sin_port = htons(broadcastPort);
	serverHint.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &serverHint.sin_addr);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct timeval tv;
	tv.tv_sec = 1;  // 1 second timeout
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	if (sock == SOCKET_ERROR) {
		std::cout << "error at socket()\n";
		return;
	}
	char sendBuf[] = "req name";
	int sendBufLen = strlen(sendBuf) + 1;	
	if (sendto(sock, sendBuf, sendBufLen, 0,(sockaddr*)&serverHint, sizeof(serverHint))== SOCKET_ERROR) {
		std::cout << "error at sendto()\n";
	}
	int max_attempt = 10;
	for (int attempt = 0; attempt < max_attempt;attempt++) {
		sockaddr_in recvAddr;
		int recvAddrLen = sizeof(recvAddr);
		char recvBuf[100]{ 0 };
		int recvBufLen = 100;
		if (recvfrom(sock, recvBuf, recvBufLen, 0, (sockaddr*)&recvAddr, &recvAddrLen) == SOCKET_ERROR) {
			continue;
		}
		max_attempt += 10;
		char tmpIPbuf[20]{ 0 };
		inet_ntop(AF_INET, &recvAddr.sin_addr, tmpIPbuf, 20);
		std::string tmpIP(tmpIPbuf);
		std::string tmpName(recvBuf);
		//add server to serversSet
		std::pair<std::string, std::string> tmp(tmpIP, tmpName);
		serversSet.insert(tmp);
	}

}

void initUI() {
	SDL_Init(SDL_INIT_VIDEO);

	const double scale = 1;
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
}

void freeUI() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void createServersList() {
	ipList = new char* [serversSet.size()];
	hostnameList = new char* [serversSet.size()];
	ipAndHostnameList = new char* [serversSet.size()];
	int i = 0;
	for (auto item : serversSet) {
		ipList[i] = new char[item.first.length() + 1];
		strcpy(ipList[i], item.first.c_str());
		hostnameList[i] = new char[item.second.length() + 1];
		strcpy(hostnameList[i], item.second.c_str());
		ipAndHostnameList[i] = new char[item.first.length() + item.second.length() + 3];
		strcpy(ipAndHostnameList[i], item.second.c_str());
		strcat(ipAndHostnameList[i], " - ");
		strcat(ipAndHostnameList[i], item.first.c_str());
		i++;
	}
}

void displayConnectPanel() {

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(screenRect.w, screenRect.h));

    if (ImGui::Begin("Connect to server", NULL, ImGuiWindowFlags_NoResize)) {

		ImGui::Text("Server's IP address:");

		ImGui::InputText("##IP", ip, IM_ARRAYSIZE(ip));

		if (ImGui::Button("Discover Servers")) {
			broadcastC(ip);
			createServersList();
			discoverState = DiscoverState::SUCCESS;
			std::cout << "Active servers:\n";
			for (auto p : serversSet) {
				std::cout << p.first << " : " << p.second << "\n";
			}
		}

		if (ImGui::Button("Exit")) {
			state = State::QUIT;
		}

		if (discoverState == DiscoverState::SUCCESS) {
			ImGui::Combo("##MyDynamicCombo", &currentItem, ipAndHostnameList, static_cast<int>(serversSet.size()));

			if (ImGui::Button("Connect")) {
				strcpy(ip, ipList[currentItem]);
				if (initClientSocket(imageSocket, ip, imagePort)) {
					connectState = ConnectionState::SUCCESS;
				}
				else {
					connectState = ConnectionState::FAIL;
				}
			}

			if (connectState == ConnectionState::FAIL) {
				ImGui::Text("Invalid IP address!");
			}
			else if (connectState == ConnectionState::SUCCESS) {
				state = State::INIT_CONTENT;
			}
		}
		
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

	SDL_RenderPresent(renderer);
}

void renderControlPanel() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(200, 100));
    if (ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_NoResize)) {
        if (ImGui::Button("Exit")) {
			state = State::QUIT;
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void renderImage(cv::Mat image) {
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image.data, image.cols, image.rows, image.channels() * 8, image.step, 0xff0000, 0x00ff00, 0x0000ff, 0);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, NULL, &screenRect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	std::this_thread::sleep_for(std::chrono::milliseconds(0));
}

void receiveAndDisplayImage() {
    while (state != State::QUIT) {
		auto start = std::chrono::high_resolution_clock::now();

		cv::Mat image = receiveImage();

        renderImage(image);

		renderControlPanel();

		SDL_RenderPresent(renderer);

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::cout << "FPS: " << 1000.0 / duration.count() << "\n";
	}
}