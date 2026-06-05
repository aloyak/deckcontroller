#ifdef CONTROLLER_DATA_H
#define CONTROLLER_DATA_H

#include <stdint.h>

struct ControllerState 
{
    uint32_t buttons;
    int16_t leftX;
    int16_t leftY;
    int16_t rightX;
    int16_t rightY;
    uint8_t leftTrigger;
    uint8_t rightTrigger;
}

#endif