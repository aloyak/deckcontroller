#include "input.h"

#include <algorithm>

namespace {
uint8_t TriggerAxisToByte(int value) {
    if (value <= 0) {
        return 0;
    }

    return static_cast<uint8_t>(std::clamp((value * 255) / 32767, 0, 255));
}
}

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
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST)  ? (1 << 1) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST)  ? (1 << 2) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH) ? (1 << 3) : 0);

    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)  ? (1 << 4) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) ? (1 << 5) : 0);

    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK)  ? (1 << 6) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK) ? (1 << 7) : 0);

    // menu
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK)  ? (1 << 12) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START) ? (1 << 14) : 0);

    state.leftX  = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    state.leftY  = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    state.rightX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    state.rightY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);

    state.leftTrigger  = TriggerAxisToByte(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
    state.rightTrigger = TriggerAxisToByte(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));

    // Pack D-pad directions into the buttons bitmask (bits 8-11)
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP)    ? (1 << 8)  : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)  ? (1 << 9)  : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT)  ? (1 << 10) : 0);
    state.buttons |= (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) ? (1 << 11) : 0);

    return state;
}

void InputManager::OpenExisting() {
    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    if (gamepads && count > 0) {
        gamepad = SDL_OpenGamepad(gamepads[0]);
    }
    SDL_free(gamepads); 
}