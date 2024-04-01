#include "gpio.h"

Gpio::Gpio (int pinNumber) {
    this->pin = pinNumber;
}

Pin::Pin () {
    this->pins = {
        Gpio(0),
        Gpio(1),
        Gpio(2),
        Gpio(3),
        Gpio(4),
        Gpio(5),
        Gpio(9),
        Gpio(10),
        Gpio(12),
        Gpio(13),
        Gpio(14),
        Gpio(15),
        Gpio(16),
    };
}

void Pin::use (int pinNumber) {
    for (auto pin : this->pins) {
        if (pin.pin == pinNumber) {
            pin.isUsed = true;
            break;
        }
    }
}

bool Pin::isUsed (int pinNumber) {
    for (auto pin : this->pins) {
        if (pin.pin == pinNumber)
            return pin.isUsed;
    }

    return false;
}

Gpio& Pin::get (int pinNumber) {
    for (unsigned int i = 0; i < this->pins.size(); i++) {
        if (this->pins[i].pin == pinNumber)
            return this->pins[i];
    }

    return this->pins[0];
}