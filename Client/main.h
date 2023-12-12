#pragma once
#include <SDL.h>
#include <thread>
#include <mutex>
#include "UI.h"

extern int screenWidth, screenHeight;
extern int serverScreenWidth, serverScreenHeight;

extern char ip[16];

extern SDL_Event event;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SOCKET imageSocket, mouseSocket, keyboardSocket;
extern SDL_Rect screenRect;

#define imagePort 55555
#define mousePort 55556
#define keyboardPort 55557

extern State state;
extern ConnectionState connectState;
