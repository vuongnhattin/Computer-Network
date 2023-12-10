#include "Mouse.h"
#include <SDL.h>
#include <WinSock2.h>
#include <iostream>
#include "main.h"

const int BUF_SIZE = 20;

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
    std::cout << std::endl;
}
LRESULT CALLBACK MouseHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;
        char buffer[BUF_SIZE];
        std::cout << wParam << std::endl;
        MouseEvent event;

        switch (wParam) {
        case WM_LBUTTONDOWN:
            event.leftMouseDown = true;
            break;
        case WM_LBUTTONUP:
            event.leftMouseDown = false;
            break;
        case WM_RBUTTONDOWN:
            event.rightMouseDown = true;
            break;
        case WM_RBUTTONUP:
            event.rightMouseDown = false;
            break;
        case WM_MBUTTONDOWN:
            event.middleMouseDown = true;
            break;
        case WM_MBUTTONUP:
            event.middleMouseDown = false;
            break;
        case WM_MOUSEMOVE:
            event.move = true;
            event.mouseX = ms->pt.x;
            event.mouseY = ms->pt.y;
            Sleep(10);
            break;
        case WM_MOUSEWHEEL:
            event.mouseWheelDelta = GET_WHEEL_DELTA_WPARAM(ms->mouseData);
            break;
        case WM_LBUTTONDBLCLK:
            event.doubleClick = true;
            break;
        default:
            break;
        }

        // Serialize the event and send it to the server
        serializeEvent(event, buffer);

        int sent;
        do {
            sent = send(mouseSocket, buffer, BUF_SIZE, 0);
            printEvent(event);
            if (sent == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cout << "Send failed! " << error << std::endl;
                    return -1;
                }
            }
        } while (sent == SOCKET_ERROR);

        // Reset the state of the modifier keys if they are not being held down
        if (event.leftMouseDown) {
            event.leftMouseDown = false;
        }
        if (event.rightMouseDown) {
            event.rightMouseDown = false;
        }
        if (event.middleMouseDown) {
            event.middleMouseDown = false;
        }
        if (event.doubleClick) {
            event.doubleClick = false;
        }
        if (event.move) {
            event.move = false;
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
    while (state != State::QUIT && GetMessage(&msg, NULL, 0, 0) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(MouseHookHandle);
}