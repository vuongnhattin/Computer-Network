#include "UI.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_impl_sdl2.h"
#include "Image.h"
#include "main.h"
#include "Socket.h"

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
            if (initSocket(clientSocket, ip, imagePort)) {
				connected = true;
			}
            else {
				validIP = false;
			}
		}
		if (!validIP) {
			ImGui::Text("Invalid IP address");
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
		cv::Mat image = receiveImage(clientSocket);
        renderImage(image);
		renderControlPanel();
		SDL_RenderPresent(renderer);
	}
}