#include "Mouse.h"
#include <SDL.h>
#include <WinSock2.h>
#include <iostream>
#include "main.h"
#include <chrono>
#include <thread>

const int BUF_SIZE = 20;
int LCD = -1;
MouseEvent mouseEvent;

void serializeEvent(const MouseEvent& event, char* buffer) {
    memcpy(&buffer[0], &(event.mouseX), sizeof(event.mouseX));
    memcpy(&buffer[4], &(event.mouseY), sizeof(event.mouseY));
    memcpy(&buffer[8], &(event.mouseWheelDelta), sizeof(event.mouseWheelDelta));
    buffer[12] = event.leftMouseDown;
    buffer[13] = event.rightMouseDown;
    buffer[14] = event.middleMouseDown;
    buffer[15] = event.doubleClick;
    buffer[16] = event.move;
}

HHOOK MouseHookHandle;
void printEvent(const MouseEvent& event) {
    std::cout << "Mouse X: " << event.mouseX << std::endl;
    std::cout << "Mouse Y: " << event.mouseY << std::endl;
    std::cout << "Mouse Wheel Delta: " << event.mouseWheelDelta << std::endl;
    std::cout << "Left Mouse Down: " << event.leftMouseDown << std::endl;
    std::cout << "Right Mouse Down: " << event.rightMouseDown << std::endl;
    std::cout << "Middle Mouse Down: " << event.middleMouseDown << std::endl;
    std::cout << "Double Click: " << event.doubleClick << std::endl;
    std::cout << "Move: " << event.move << std::endl;
}
LRESULT CALLBACK MouseHook(int code, WPARAM wParam, LPARAM lParam) {
    if (uiState != UIState::DISPLAY_IMAGE) {
        PostQuitMessage(0);
        UnhookWindowsHookEx(MouseHookHandle);
        return 0;
    }
    const double screenWidthRatio = serverScreenWidth * 1.0 / screenWidth;
    const double screenHeightRatio = serverScreenHeight * 1.0 / screenHeight;
    if (code == HC_ACTION) {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;
        char buffer[BUF_SIZE];
        std::cout << wParam << std::endl;
        switch (wParam) {
        case WM_LBUTTONDOWN:
            mouseEvent.leftMouseDown = true;
            break;
        case WM_LBUTTONUP:
            mouseEvent.leftMouseDown = false;
            break;
        case WM_RBUTTONDOWN:
            mouseEvent.rightMouseDown = true;
            break;
        case WM_RBUTTONUP:
            mouseEvent.rightMouseDown = false;
            break;
        case WM_MBUTTONDOWN:
            mouseEvent.middleMouseDown = true;
            break;
        case WM_MBUTTONUP:
            mouseEvent.middleMouseDown = false;
            break;
        case WM_MOUSEMOVE:
            mouseEvent.move = true;
            mouseEvent.mouseX = (DWORD)(ms->pt.x * screenWidthRatio);
            mouseEvent.mouseY = (DWORD)(ms->pt.y * screenHeightRatio);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            break;
        case WM_MOUSEWHEEL:
            mouseEvent.mouseWheelDelta = GET_WHEEL_DELTA_WPARAM(ms->mouseData);
            break;
        case WM_LBUTTONDBLCLK:
            mouseEvent.doubleClick = true;
            break;
        default:
            break;
        }
        // Serialize the event and send it to the server
        serializeEvent(mouseEvent, buffer);

        int sent;
        do {
            sent = send(mouseSocket, buffer, BUF_SIZE, 0);
            //printEvent(mouseEvent);
            std::cout << "mouse event detected.\n";
            if (sent == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cout << "Send failed! " << error << std::endl;
                    return -1;
                }
            }
        } while (sent == SOCKET_ERROR);

        // Reset the state of the modifier keys if they are not being held down
        if (mouseEvent.doubleClick) {
            mouseEvent.doubleClick = false;
        }
        if (mouseEvent.move) {
            mouseEvent.move = false;
        }
        if (mouseEvent.mouseWheelDelta != 0) {
			mouseEvent.mouseWheelDelta = 0;
		}
    }

    return CallNextHookEx(MouseHookHandle, code, wParam, lParam);
}

void sendMouseEvents() {
	MouseHookHandle = SetWindowsHookEx(WH_MOUSE_LL, MouseHook, NULL, 0);
    if (MouseHookHandle == NULL) {
		std::cout << "Mouse hook failed!\n";
		return;
	}

	MSG msg;
    while (uiState == UIState::DISPLAY_IMAGE && GetMessage(&msg, NULL, 0, 0) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    std::cout << "shut down mouse thread.\n";
	//UnhookWindowsHookEx(MouseHookHandle);
}