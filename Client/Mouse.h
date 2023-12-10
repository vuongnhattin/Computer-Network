#pragma once
#include <WinSock2.h>
#include <SDL.h>

struct MouseEvent {
    DWORD mouseX = 0;
    DWORD mouseY = 0;
    DWORD mouseWheelDelta = 0;
    bool leftMouseDown = false;
    bool rightMouseDown = false;
    bool middleMouseDown = false;
    bool doubleClick = false;
    bool move = false;
};

void serializeEvent(const MouseEvent& event, char* buffer);
void printEvent(const MouseEvent& event);
LRESULT CALLBACK MouseHook(int code, WPARAM wParam, LPARAM lParam);
void sendMouseEvents();