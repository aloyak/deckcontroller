#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL3 could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // created a basic window but final version should just dim the screen fully
    // TODO: get decks input

    SDL_Window* window = SDL_CreateWindow("Deck Client", 800, 600, 0); // SDL_WINDOW_MAXIMIZED?
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        
        SDL_Delay(32); 
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}