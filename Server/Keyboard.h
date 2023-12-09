#pragma once
#include <WinSock2.h>

struct KeyEvent {
    bool isKeyDown;
    unsigned char keyCode;
    bool ctrlPressed;
    bool shiftPressed;
    bool altPressed;
    bool winPressed;
};

void simulateKeyEvent(const char* serializedData);
void handleKeyboard(SOCKET clientSocket);