#include "UI.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_impl_sdl2.h"
#include "Image.h"
#include "main.h"
#include "Socket.h"
#include <chrono>

using namespace std::chrono;

void initUI() {
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
}

void cleanUpUI() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void renderLoginPanel() {
	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(screenRect.w, screenRect.h));
    if (ImGui::Begin("Connect to server", NULL, ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Server's IP address:");
		ImGui::InputText("##IP", ip, IM_ARRAYSIZE(ip));
        if (ImGui::Button("Connect")) {
            if (initClientSocket(imageSocket, ip, imagePort)) {
				connected = true;
			}
            else {
				validIP = false;
			}
		}
		if (!validIP) {
			ImGui::Text("Invalid IP address!");
		}
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void renderControlPanel() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(200, 100));
    if (ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_NoResize)) {
        if (ImGui::Button("Exit")) {
            quit = true;
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void displayContent() {
    while (!quit) {
		auto start = high_resolution_clock::now();

		cv::Mat image = receiveImage(imageSocket);
        renderImage(image);
		renderControlPanel();
		SDL_RenderPresent(renderer);

		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		std::cout << "FPS: " << 1000.0 / duration.count() << "\n";
	}
}