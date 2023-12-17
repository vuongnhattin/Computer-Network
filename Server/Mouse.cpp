#include "Mouse.h"
#include "main.h"
#include <iostream>

const int BUF_SIZE = 20;
MouseEvent mouseEvent;
bool prevLeft = 0;
bool prevRight = 0;
bool prevMiddle = 0;

// Function to simulate a key event
void simulateMouseEvent(const char* serializedData) {
    // Create a keyboard input event
    INPUT ip;
    ZeroMemory(&ip, sizeof(INPUT));
    ip.type = INPUT_MOUSE;
    prevLeft = mouseEvent.leftMouseDown;
    prevRight = mouseEvent.rightMouseDown;
    prevMiddle = mouseEvent.middleMouseDown;
    // Parse the serialized data
    memcpy(&mouseEvent, serializedData, sizeof(MouseEvent));

    ip.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    ip.mi.dx = (mouseEvent.mouseX * 65535) / GetSystemMetrics(SM_CXSCREEN);
    ip.mi.dy = (mouseEvent.mouseY * 65535) / GetSystemMetrics(SM_CYSCREEN);

    // Simulate the key event
    if (mouseEvent.leftMouseDown != prevLeft) {
        if (mouseEvent.leftMouseDown)
            ip.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
        else
            ip.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
    }
    if (mouseEvent.rightMouseDown != prevRight) {
        if (mouseEvent.rightMouseDown)
            ip.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
        else
            ip.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
    }
    if (mouseEvent.middleMouseDown != prevMiddle) {
        if (mouseEvent.middleMouseDown)
            ip.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
        else
            ip.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
    }
    if (mouseEvent.mouseWheelDelta != 0) {
        ip.mi.dwFlags |= MOUSEEVENTF_WHEEL;
        ip.mi.mouseData = mouseEvent.mouseWheelDelta;
    }
    if (mouseEvent.move) {
        ip.mi.dwFlags |= MOUSEEVENTF_MOVE;
    }
    SendInput(1, &ip, sizeof(INPUT));
}

void handleMouse(SOCKET acceptMouseSocket) {
	char buff[BUF_SIZE];
    while (uiState == UIState::DISPLAY_CONTROL_PANEL) {
		recv(acceptMouseSocket, buff, BUF_SIZE, 0);
		simulateMouseEvent(buff);
	}
    std::cout << "shut down handle mouse.\n";
}