#include "UI.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_impl_sdl2.h"
#include "Image.h"
#include "main.h"
#include "Socket.h"
#include <chrono>

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

void displayConnectPanel() {
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
				connectState = ConnectionState::SUCCESS;
			}
            else {
				connectState = ConnectionState::FAIL;
			}
		}
		if (ImGui::Button("Exit")) {
			state = State::QUIT;
		}
		if (connectState == ConnectionState::FAIL) {
			ImGui::Text("Invalid IP address!");
		}
		else if (connectState == ConnectionState::SUCCESS) {
			state = State::INIT_CONTENT;
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