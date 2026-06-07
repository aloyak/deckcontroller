#include "linux/driver.h"

#include <cstring>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
void RequireIoctl(int result, const char* message) {
    if (result < 0) {
        throw std::runtime_error(message);
    }
}
}

Driver::Driver() {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) throw std::runtime_error("Cannot open /dev/uinput");

    RequireIoctl(ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY), "Cannot enable key events");
    RequireIoctl(ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS), "Cannot enable absolute axes");

    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_A), "Cannot register BTN_A");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_B), "Cannot register BTN_B");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_X), "Cannot register BTN_X");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_Y), "Cannot register BTN_Y");

    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TL), "Cannot register BTN_TL"); // LB
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TR), "Cannot register BTN_TR"); // RB

    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBL), "Cannot register BTN_THUMBL"); // L3
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBR), "Cannot register BTN_THUMBR"); // R3

    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_UP), "Cannot register BTN_DPAD_UP");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_DOWN), "Cannot register BTN_DPAD_DOWN");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_LEFT), "Cannot register BTN_DPAD_LEFT");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT), "Cannot register BTN_DPAD_RIGHT");

    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_SELECT), "Cannot register BTN_SELECT"); // MENU
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_START), "Cannot register BTN_START");
    RequireIoctl(ioctl(uinput_fd, UI_SET_KEYBIT, BTN_MODE), "Cannot register BTN_MODE"); // Steam / guide

    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_X), "Cannot register ABS_X");
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Y), "Cannot register ABS_Y");
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RX), "Cannot register ABS_RX");
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RY), "Cannot register ABS_RY");
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Z), "Cannot register ABS_Z"); // LT
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RZ), "Cannot register ABS_RZ"); // RT
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_HAT0X), "Cannot register ABS_HAT0X");
    RequireIoctl(ioctl(uinput_fd, UI_SET_ABSBIT, ABS_HAT0Y), "Cannot register ABS_HAT0Y");

    uinput_setup usetup{};
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x045e; 
    usetup.id.product = 0x028e;  // Xbox 360 controller for compatibility
    std::strncpy(usetup.name, "Steam Deck Controller", sizeof(usetup.name) - 1);

    RequireIoctl(ioctl(uinput_fd, UI_DEV_SETUP, &usetup), "Cannot setup uinput device");

    uinput_abs_setup abs{};
    abs.absinfo.minimum = -32767;
    abs.absinfo.maximum =  32767;
    abs.absinfo.flat    =  128;    // dead zone
    abs.absinfo.fuzz    =  16;
    abs.absinfo.resolution = 0;

    for (int axis : {ABS_X, ABS_Y, ABS_RX, ABS_RY}) {
        abs.code = axis;
        RequireIoctl(ioctl(uinput_fd, UI_ABS_SETUP, &abs), "Cannot setup stick axis");
    }

    abs.absinfo.minimum = 0;
    abs.absinfo.maximum = 255;
    abs.absinfo.flat    = 0;
    abs.absinfo.fuzz    = 0;
    abs.code = ABS_Z;
    RequireIoctl(ioctl(uinput_fd, UI_ABS_SETUP, &abs), "Cannot setup left trigger axis");
    abs.code = ABS_RZ;
    RequireIoctl(ioctl(uinput_fd, UI_ABS_SETUP, &abs), "Cannot setup right trigger axis");

    // D-pad: -1, 0, 1 range.
    abs.absinfo.minimum = -1;
    abs.absinfo.maximum = 1;
    abs.absinfo.flat    = 0;
    abs.absinfo.fuzz    = 0;
    abs.code = ABS_HAT0X;
    RequireIoctl(ioctl(uinput_fd, UI_ABS_SETUP, &abs), "Cannot setup dpad horizontal axis");
    abs.code = ABS_HAT0Y;
    RequireIoctl(ioctl(uinput_fd, UI_ABS_SETUP, &abs), "Cannot setup dpad vertical axis");

    RequireIoctl(ioctl(uinput_fd, UI_DEV_CREATE), "Cannot create uinput device");
}

Driver::~Driver() {
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
}

void Driver::Action(int type, int code, int value) {
    input_event ev{};
    ev.type  = type;
    ev.code  = code;
    ev.value = value;
    write(uinput_fd, &ev, sizeof(ev));
}

void Driver::Run(ControllerSnapshot snapshot) {
    const ControllerState currentState = snapshot.connected ? snapshot.state : ControllerState{};
    bool changed = false;

    auto emitKey = [&](int bit, int code) {
        const int value = (currentState.buttons >> bit) & 1U;
        const int previous = (lastState.buttons >> bit) & 1U;
        if (!hasLastState || value != previous) {
            Action(EV_KEY, code, value);
            changed = true;
        }
    };

    auto emitAbs = [&](int code, int value, int previous) {
        if (!hasLastState || value != previous) {
            Action(EV_ABS, code, value);
            changed = true;
        }
    };

    emitKey(0, BTN_A);
    emitKey(1, BTN_B);
    emitKey(2, BTN_X);
    emitKey(3, BTN_Y);

    emitKey(4, BTN_TL);
    emitKey(5, BTN_TR);

    emitKey(6, BTN_THUMBL);
    emitKey(7, BTN_THUMBR);

    emitKey(8, BTN_DPAD_UP);
    emitKey(9, BTN_DPAD_DOWN);
    emitKey(10, BTN_DPAD_LEFT);
    emitKey(11, BTN_DPAD_RIGHT);

    emitKey(12, BTN_SELECT);
    emitKey(13, BTN_MODE);
    emitKey(14, BTN_START);

    emitAbs(ABS_X, currentState.leftX, lastState.leftX);
    emitAbs(ABS_Y, currentState.leftY, lastState.leftY);
    emitAbs(ABS_RX, currentState.rightX, lastState.rightX);
    emitAbs(ABS_RY, currentState.rightY, lastState.rightY);
    emitAbs(ABS_Z, currentState.leftTrigger, lastState.leftTrigger);
    emitAbs(ABS_RZ, currentState.rightTrigger, lastState.rightTrigger);

    if (changed) {
        Action(EV_SYN, SYN_REPORT, 0);
    }

    lastState = currentState;
    hasLastState = true;
}