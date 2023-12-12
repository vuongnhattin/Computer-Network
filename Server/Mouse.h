#pragma once
#include <WinSock2.h>

struct MouseEvent {
    DWORD mouseX = 0;
    DWORD mouseY = 0;
    DWORD mouseWheelDelta = 0;
    bool leftMouseDown = 0;
    bool rightMouseDown = 0;
    bool middleMouseDown = 0;
    bool doubleClick = 0;
    bool move = 0;
};

void simulateMouseEvent(const char* serializedData);
void handleMouse(SOCKET acceptMouseSocket);