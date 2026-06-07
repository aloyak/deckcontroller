#pragma once

#include <atomic>
#include "state.h"

void RunNetworkLoop(std::atomic<bool>& running, SharedState& shared, int port=8080);
