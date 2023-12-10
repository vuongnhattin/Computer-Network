#include "Keyboard.h"
#include <WinSock2.h>
#include "main.h"
#include <iostream>

const int BUF_SIZE = 20;

// Function to simulate a key event
void simulateKeyEvent(const char* serializedData) {
    // Create a keyboard input event
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    // Parse the serialized data
    KeyEvent event;
    event.isKeyDown = serializedData[0];
    event.ctrlPressed = serializedData[1];
    event.shiftPressed = serializedData[2];
    event.altPressed = serializedData[3];
    event.winPressed = serializedData[4];
    memcpy(&(event.keyCode), &serializedData[5], sizeof(event.keyCode));

    // Simulate the key event
    if (event.ctrlPressed) {
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = event.isKeyDown ? 0 : KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }
    if (event.shiftPressed) {
        ip.ki.wVk = VK_SHIFT;
        ip.ki.dwFlags = event.isKeyDown ? 0 : KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }
    if (event.altPressed) {
        ip.ki.wVk = VK_MENU;
        ip.ki.dwFlags = event.isKeyDown ? 0 : KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }
    if (event.winPressed) {
        ip.ki.wVk = VK_LWIN;
        ip.ki.dwFlags = event.isKeyDown ? 0 : KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }
    ip.ki.wVk = event.keyCode;
    ip.ki.dwFlags = event.isKeyDown ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));

    // Simulate releasing the modifier keys
    if (event.ctrlPressed) {
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // Key up
        SendInput(1, &ip, sizeof(INPUT));
    }
    if (event.shiftPressed) {
        ip.ki.wVk = VK_SHIFT;
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // Key up
        SendInput(1, &ip, sizeof(INPUT));
    }
    if (event.altPressed) {
        ip.ki.wVk = VK_MENU;
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // Key up
        SendInput(1, &ip, sizeof(INPUT));
    }
    if (event.winPressed) {
        ip.ki.wVk = VK_LWIN;
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // Key up
        SendInput(1, &ip, sizeof(INPUT));
    }
}

void handleKeyboard(SOCKET acceptSocket) {
	char buff[BUF_SIZE];
    while (!quit) {
		recv(acceptSocket, buff, BUF_SIZE, 0);
        std::cout << "Received data: " << buff << "\n";
		simulateKeyEvent(buff);
	}
}