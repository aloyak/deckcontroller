#pragma once

#include "state.h"

#include <ViGEm/Client.h>

// WIP driver

class Driver {
public:
    Driver();
    ~Driver();

    void Run(ControllerSnapshot snapshot);
private:
    PVIGEM_CLIENT client;
    PVIGEM_TARGET target;
};