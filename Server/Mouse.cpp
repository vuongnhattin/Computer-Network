#include "Mouse.h"
#include "main.h"

const int BUF_SIZE = 20;

// Function to simulate a key event
void simulateMouseEvent(const char* serializedData) {
    // Create a keyboard input event
    INPUT ip;
    ZeroMemory(&ip, sizeof(INPUT));
    ip.type = INPUT_MOUSE;

    // Parse the serialized data
    MouseEvent event;
    memcpy(&event, serializedData, sizeof(MouseEvent));

    ip.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    ip.mi.dx = (event.mouseX * 65535) / GetSystemMetrics(SM_CXSCREEN);
    ip.mi.dy = (event.mouseY * 65535) / GetSystemMetrics(SM_CYSCREEN);

    // Simulate the key event
    if (event.leftMouseDown)
        ip.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
    if (event.rightMouseDown)
        ip.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
    if (event.middleMouseDown)
        ip.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
    if (event.mouseWheelDelta != 0) {
        ip.mi.dwFlags |= MOUSEEVENTF_WHEEL;
        ip.mi.mouseData = event.mouseWheelDelta;
    }
    if (event.move) {
        ip.mi.dwFlags |= MOUSEEVENTF_MOVE;
    }

    SendInput(1, &ip, sizeof(INPUT));

    ip.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    ip.mi.dx = (event.mouseX * 65535) / GetSystemMetrics(SM_CXSCREEN);
    ip.mi.dy = (event.mouseY * 65535) / GetSystemMetrics(SM_CYSCREEN);
    if (event.leftMouseDown)
        ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    if (event.rightMouseDown)
        ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    if (event.middleMouseDown)
        ip.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void handleMouse(SOCKET acceptMouseSocket) {
	char buff[BUF_SIZE];
    while (state != State::QUIT) {
		recv(acceptMouseSocket, buff, BUF_SIZE, 0);
		simulateMouseEvent(buff);
	}
}