#include <SDL3/SDL.h>
#include "network.h"
#include "input.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    NetworkClient net(8080);
    net.EnableBroadcast();
    InputManager input;

    bool connected = false;
    sockaddr_in pcAddr = {}; // Filled when PC responds
    
    while (true) {
        if (!connected) {
            Packet p = {PACKET_BROADCAST, "DeckID"};
            net.SendBroadcast(p);
        } else {
            Packet p = {PACKET_INPUT_STATE, "DeckID", input.GetCurrentState()};
            net.Send(p, pcAddr);
        }
        SDL_Delay(16);
    }
    return 0;
}