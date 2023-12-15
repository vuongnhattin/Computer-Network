#pragma once
#include <SDL.h>
#include "UI.h"

extern const int screenWidth, screenHeight;

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Rect screenRect;

#define DEPTH 4
#define broadcastPort 55554
#define imagePort 55555
#define mousePort 55556
#define keyboardPort 55557

extern char ip[16];

extern SOCKET imageSocket, acceptImageSocket;
extern SOCKET mouseSocket, acceptMouseSocket;
extern SOCKET keyboardSocket, acceptKeyboardSocket;

extern State state;
extern BindingState bindingState;
extern ConnectionState connectionState;