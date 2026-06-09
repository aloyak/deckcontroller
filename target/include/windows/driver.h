#pragma once
 
#include "state.h"
 
// vJoy public header — comes with the vJoy SDK (drop public.h next to this file)
// SDK: https://github.com/jshafer817/vJoy  or the installer's SDK folder
#include "public.h"

// WIP driver

class Driver {
public:
    explicit Driver(UINT deviceID = 1);
    ~Driver();

    Driver(const &Driveer) = delete;
    Driver& operator=(const &Driver) = delete;

    void Run(ControllerSnapshot snapshot);
private:
    UINT deviceID;
};