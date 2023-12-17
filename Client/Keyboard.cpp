#include "Keyboard.h"
#include <WinSock2.h>
#include "main.h"
#include <iostream>
#include <thread>

using namespace std::chrono;

const int BUF_SIZE = 20;

void serializeEvent(const KeyEvent& event, char* buffer) {

    buffer[0] = event.isKeyDown;
    buffer[1] = event.ctrlPressed;
    buffer[2] = event.shiftPressed;
    buffer[3] = event.altPressed;
    buffer[4] = event.winPressed;

    memcpy(&buffer[5], &(event.keyCode), sizeof(event.keyCode));
}

HHOOK keyboardHookHandle;

LRESULT CALLBACK keyboardHook(int code, WPARAM wParam, LPARAM lParam) {
    if (uiState != UIState::DISPLAY_IMAGE) {
        PostQuitMessage(0);
        UnhookWindowsHookEx(keyboardHookHandle);
        std::cout << "unhooked keyboard\n";
        return 0;
    }
    static bool ctrlPressed = false;
    static bool shiftPressed = false;
    static bool altPressed = false;
    static bool winPressed = false;

    if (code == HC_ACTION) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        char buffer[BUF_SIZE];

        KeyEvent event;
        if (kb->flags & LLKHF_INJECTED) {
            return CallNextHookEx(keyboardHookHandle, code, wParam, lParam);
        }
        event.isKeyDown = (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN);
        event.keyCode = kb->vkCode;

        // Check if the key event is for a modifier key
        switch (event.keyCode) {
        case VK_CONTROL:
            if (event.isKeyDown && !ctrlPressed) {
                ctrlPressed = TRUE;
            }
            else if (!event.isKeyDown && ctrlPressed) {
                ctrlPressed = FALSE;
                event.ctrlPressed = TRUE;
                serializeEvent(event, buffer);
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid race condition
            }
            break;
        case VK_SHIFT:
            if (event.isKeyDown && !shiftPressed) {
                shiftPressed = TRUE;
            }
            else if (!event.isKeyDown && shiftPressed) {
                shiftPressed = FALSE;
                event.shiftPressed = TRUE;
                serializeEvent(event, buffer);
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid race condition
            }
            break;
        case VK_MENU: // Alt key
            if (event.isKeyDown && !altPressed) {
                altPressed = TRUE;
            }
            else if (!event.isKeyDown && altPressed) {
                altPressed = FALSE;
                event.altPressed = TRUE;
                serializeEvent(event, buffer);
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid race condition
            }
            break;
        case VK_LWIN: // Windows key
        case VK_RWIN:
            if (event.isKeyDown && !winPressed) {
                winPressed = TRUE;
            }
            else if (!event.isKeyDown && winPressed) {
                winPressed = FALSE;
                event.winPressed = TRUE;
                serializeEvent(event, buffer);
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid race condition
            }
            break;
        }
        // Include the state of the modifier keys in the KeyEvent
        event.ctrlPressed = ctrlPressed;
        event.shiftPressed = shiftPressed;
        event.altPressed = altPressed;
        event.winPressed = winPressed;
        serializeEvent(event, buffer);

        int sent;
        do {
            sent = send(keyboardSocket, buffer, BUF_SIZE, 0);
            std::cout << "keyboard event detected.\n";
            if (sent == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cout << "Send failed! " << error << std::endl;
                    return -1;
                }
            }
        } while (sent == SOCKET_ERROR);

        // Reset the state of the modifier keys if they are not being held down
        if (!event.isKeyDown) {
            if (event.ctrlPressed) {
                event.ctrlPressed = TRUE;
                serializeEvent(event, buffer);
            }
            if (event.shiftPressed) {
                event.shiftPressed = TRUE;
                serializeEvent(event, buffer);
            }
            if (event.altPressed) {
                event.altPressed = TRUE;
                serializeEvent(event, buffer);
            }
            if (event.winPressed) {
                event.winPressed = TRUE;
                serializeEvent(event, buffer);
            }
        }
        return 1;
    }

    return CallNextHookEx(keyboardHookHandle, code, wParam, lParam);
}

void sendKeyboardEvents() {
	keyboardHookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHook, NULL, 0);
    if (keyboardHookHandle == NULL) {
		std::cout << "Failed to set keyboard hook!" << std::endl;
		return;
	}

	MSG msg;
    while (uiState == UIState::DISPLAY_IMAGE && GetMessage(&msg, NULL, 0, 0) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    std::cout << "shut down keyboard thread\n";
	//UnhookWindowsHookEx(keyboardHookHandle);
}
