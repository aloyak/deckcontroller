#include "windows/driver.h"

#include <stdexcept>

Driver::Driver() {
    client = vigem_alloc();
    if (client == nullptr) {
        throw std::runtime_error("Failed to allocate ViGEm client");

    VIGEM_ERROR err = vigem_connect(client);
    if (!VIGEM_SUCCESS(err)) {
        vigem_free(client);
        throw std::runtime_error("Failed to connect to ViGEm bus");
    }

    target = vigem_target_x360_alloc();
    if (target == nullptr) {
        vigem_free(client);
        throw std::runtime_error("Failed to allocate ViGEm target");

    }

    ret = vigem_target_add(client, target);
    if (!VIGEM_SUCCESS(ret))
        throw std::runtime_error("Failed to add controller target");
}

Driver::~Driver() {
    if (target) {
        vigem_target_remove(client, target);
        vigem_target_free(target);
    }
    if (client) {
        vigem_disconnect(client);
        vigem_free(client);
    }
}

void Driver::Run(ControllerSnapshot snapshot) {
    const ControllerState& s = snapshot.connected ? snapshot.state : ControllerState{};

    XUSB_REPORT report{};

    auto btn = [&](int bit) -> bool { return (s.buttons >> bit) & 1U; };

    if (btn(0))  report.wButtons |= XUSB_GAMEPAD_A;
    if (btn(1))  report.wButtons |= XUSB_GAMEPAD_B;
    if (btn(2))  report.wButtons |= XUSB_GAMEPAD_X;
    if (btn(3))  report.wButtons |= XUSB_GAMEPAD_Y;
    if (btn(4))  report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
    if (btn(5))  report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
    if (btn(6))  report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
    if (btn(7))  report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
    if (btn(8))  report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
    if (btn(9))  report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
    if (btn(10)) report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
    if (btn(11)) report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;
    if (btn(12)) report.wButtons |= XUSB_GAMEPAD_BACK;
    if (btn(13)) report.wButtons |= XUSB_GAMEPAD_GUIDE;
    if (btn(14)) report.wButtons |= XUSB_GAMEPAD_START;

    report.sThumbLX = static_cast<SHORT>(s.leftX);
    report.sThumbLY = static_cast<SHORT>(s.leftY);
    report.sThumbRX = static_cast<SHORT>(s.rightX);
    report.sThumbRY = static_cast<SHORT>(s.rightY);

    report.bLeftTrigger  = static_cast<BYTE>(s.leftTrigger);
    report.bRightTrigger = static_cast<BYTE>(s.rightTrigger);

    vigem_target_x360_update(client, target, report);
}