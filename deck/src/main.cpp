#include <SDL3/SDL.h>
#include <string>
#include "network.h"
#include "input.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

    SDL_Window* window = SDL_CreateWindow("Deck Controller", 400, 200, 0);

    NetworkClient net(8080);
    net.EnableBroadcast();
    net.SetNonBlocking(true);
    InputManager input;

    // Open any gamepad already connected at startup
    input.OpenExisting();

    bool connected = false;
    sockaddr_in pcAddr = {}; // Filled when PC responds

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input.HandleEvent(event);
            if (event.type == SDL_EVENT_QUIT) return 0;
        }

        if (!connected) {
            Packet p = {PACKET_BROADCAST, "DeckID"};
            net.SendBroadcast(p);
            Packet reply;
            if (net.Receive(reply, pcAddr))
                connected = true;
        } else {
            ControllerState state = input.GetCurrentState();
            Packet p = {PACKET_INPUT_STATE, "DeckID", state};
            net.Send(p, pcAddr);
        }
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}