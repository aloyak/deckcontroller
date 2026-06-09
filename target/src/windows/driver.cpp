#include "driver.h"

#include <stdexcept>
#include <string>

static constexpr LONG VJOY_AXIS_MIN  = 0x0001;
static constexpr LONG VJOY_AXIS_MAX  = 0x8000;
static constexpr LONG VJOY_AXIS_MID  = (VJOY_AXIS_MAX + VJOY_AXIS_MIN) / 2; // 16384

static inline LONG StickToVJoy(int v) {
    long shifted = static_cast<long>(v) + 32768L;           // 0 – 65535
    return VJOY_AXIS_MIN + (shifted * (VJOY_AXIS_MAX - VJOY_AXIS_MIN)) / 65535L;
}

static inline LONG TriggerToVJoy(int v) {
    return VJOY_AXIS_MIN + (static_cast<long>(v) * (VJOY_AXIS_MAX - VJOY_AXIS_MIN)) / 255L;
}

static inline LONG DpadToPov(DWORD buttons) {
    bool up    = (buttons >> 8)  & 1U;
    bool down  = (buttons >> 9)  & 1U;
    bool left  = (buttons >> 10) & 1U;
    bool right = (buttons >> 11) & 1U;

    if (up   && right) return  4500;
    if (right && down) return 13500;
    if (down  && left) return 22500;
    if (left  && up)   return 31500;
    if (up)            return     0;
    if (right)         return  9000;
    if (down)          return 18000;
    if (left)          return 27000;
    return -1; // centred
}

Driver::Driver(UINT id) : deviceId(id) {
    if (!vJoyEnabled()) {
        throw std::runtime_error("vJoy driver is not enabled or not installed");
    }

    WORD verDll = 0, verDrv = 0;
    if (!DriverMatch(&verDll, &verDrv)) {
        throw std::runtime_error(
            "vJoy DLL/driver version mismatch (DLL=" + std::to_string(verDll) +
            " DRV=" + std::to_string(verDrv) + ")");
    }

    VjdStat status = GetVJDStatus(deviceId);
    switch (status) {
        case VJD_STAT_OWN:
            break;
        case VJD_STAT_FREE:
            break;
        case VJD_STAT_BUSY:
            throw std::runtime_error("vJoy device " + std::to_string(deviceId) +
                                     " is already owned by another process");
        case VJD_STAT_MISS:
            throw std::runtime_error("vJoy device " + std::to_string(deviceId) +
                                     " is not configured. Add it in vJoy Config.");
        default:
            throw std::runtime_error("vJoy device " + std::to_string(deviceId) +
                                     " returned unknown status");
    }

    if (!AcquireVJD(deviceId)) {
        throw std::runtime_error("Failed to acquire vJoy device " + std::to_string(deviceId));
    }

    ResetVJD(deviceId);
}

Driver::~Driver() {
    RelinquishVJD(deviceId);
}

void Driver::Run(ControllerSnapshot snapshot) {
    const ControllerState& s = snapshot.connected ? snapshot.state : ControllerState{};

    auto btn = [&](int bit) -> bool { return (s.buttons >> bit) & 1U; };

    for (int bit = 0; bit <= 14; ++bit) {
        SetBtn(btn(bit), deviceId, static_cast<UCHAR>(bit + 1));
    }

    SetAxis(StickToVJoy(s.leftX),  deviceId, HID_USAGE_X);   // Left  X
    SetAxis(StickToVJoy(s.leftY),  deviceId, HID_USAGE_Y);   // Left  Y
    SetAxis(StickToVJoy(s.rightX), deviceId, HID_USAGE_RX);  // Right X
    SetAxis(StickToVJoy(s.rightY), deviceId, HID_USAGE_RY);  // Right Y

    SetAxis(TriggerToVJoy(s.leftTrigger),  deviceId, HID_USAGE_Z);
    SetAxis(TriggerToVJoy(s.rightTrigger), deviceId, HID_USAGE_RZ);

    SetContPov(DpadToPov(s.buttons), deviceId, 1);
}