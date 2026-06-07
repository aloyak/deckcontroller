// Copy from deck side. TODO: Refactor to use a shared header for packet definitions

#ifndef CONTROLLER_DATA_H
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
};

enum PacketType : uint8_t
{
    PACKET_BROADCAST,
    PACKET_CONNECT,
    PACKET_INPUT_STATE
};

struct Packet
{
    PacketType type;
    char id[32];
    ControllerState state;
};

#endif