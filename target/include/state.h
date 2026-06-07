#pragma once

#include <SDL3/SDL.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

#include "data.h"

struct SharedState {
    std::mutex      mtx;
    ControllerState state = {};
    bool            connected = false;
    std::string     deckIP;
    std::string     deckID;
    uint64_t        lastPacketTime = 0;
};

struct ControllerSnapshot {
    ControllerState state = {};
    bool            connected = false;
    std::string     deckIP;
    std::string     deckID;
    uint64_t        lastPacketTime = 0;
};

inline ControllerSnapshot SnapshotState(SharedState& shared) {
    std::lock_guard<std::mutex> lock(shared.mtx);
    return {
        shared.state,
        shared.connected,
        shared.deckIP,
        shared.deckID,
        shared.lastPacketTime,
    };
}
