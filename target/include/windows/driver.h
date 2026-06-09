#pragma once

#include "state.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <public.h>
#include <vjoyinterface.h>

class Driver {
public:
    explicit Driver(UINT deviceId = 1);
    ~Driver();

    // Disabled copy — device handle is not shareable
    Driver(const Driver&) = delete;
    Driver& operator=(const Driver&) = delete;

    void Run(ControllerSnapshot snapshot);

private:
    UINT deviceId;
};