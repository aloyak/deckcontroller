#include "input.h"

void InputManager::HandleEvent(SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_ADDED && !gamepad) {
        gamepad = SDL_OpenGamepad(event.gdevice.which);
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED && gamepad) {
        if (event.gdevice.which == SDL_GetGamepadID(gamepad)) {
            SDL_CloseGamepad(gamepad);
            gamepad = nullptr;
        }
    }
}


ControllerState InputManager::GetCurrentState() {
    ControllerState state = {};
    if (!gamepad) return state;

    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH) ? (1 << 0) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST) ? (1 << 1) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST) ? (1 << 2) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH) ? (1 << 3) : 0);
    
    state.leftX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    state.leftY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    state.rightX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    state.rightY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    
    state.leftTrigger = (uint8_t)(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) / 256);
    state.rightTrigger = (uint8_t)(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 256);
    
    return state;
}