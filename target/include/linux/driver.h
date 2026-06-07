#pragma once

#include "state.h"

// using uinput to create a virtual game controller

class Driver {
public:
    Driver();
    ~Driver();

    void Run(ControllerSnapshot snapshot);
    void Action(int type, int code, int value);

private:
    int uinput_fd;
    ControllerState lastState{};
    bool hasLastState = false;
};