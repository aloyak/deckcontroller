#pragma once

#include <SDL3/SDL.h>
#include "data.h"

class InputManager {
public:
    InputManager() : gamepad(nullptr) {}
    
    void HandleEvent(SDL_Event& event);
    bool IsGamepadConnected() const { return gamepad != nullptr; }
    ControllerState GetCurrentState();

private: 
    SDL_Gamepad* gamepad;
};