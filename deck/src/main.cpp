#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Deck Client | No Gamepad Connected", 800, 600, 0);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Gamepad* gamepad = nullptr;
    bool quit = false;
    SDL_Event event;

    float last_axis_x = 0.0f;
    bool south_pressed = false;

    while (!quit) {
        bool status_changed = false;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            } else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                if (!gamepad) {
                    gamepad = SDL_OpenGamepad(event.gdevice.which);
                    status_changed = true;
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                if (gamepad && event.gdevice.which == SDL_GetGamepadID(gamepad)) {
                    SDL_CloseGamepad(gamepad);
                    gamepad = nullptr;
                    status_changed = true;
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                    south_pressed = true;
                    status_changed = true;
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                    south_pressed = false;
                    status_changed = true;
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
                if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
                    last_axis_x = event.gaxis.value / 32267.0f;
                    if (last_axis_x > 1.0f) last_axis_x = 1.0f;
                    if (last_axis_x < -1.0f) last_axis_x = -1.0f;
                    status_changed = true;
                }
            }
        }

        if (status_changed) {
            std::string title = "Deck Client | ";
            if (gamepad) {
                title += "Connected ID: " + std::to_string(SDL_GetGamepadID(gamepad));
                title += " | X-Axis: " + std::to_string(last_axis_x);
                title += " | Button A: " + std::string(south_pressed ? "PRESSED" : "RELEASED");
            } else {
                title += "Gamepad Disconnected";
            }
            SDL_SetWindowTitle(window, title.c_str());
        }
        
        SDL_Delay(1);
    }

    if (gamepad) {
        SDL_CloseGamepad(gamepad);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}