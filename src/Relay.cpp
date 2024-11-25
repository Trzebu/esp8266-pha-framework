#include "Relay.h"

Relay::Relay (String name, Gpio& gpio, bool invertState) {
    this->name = name;
    this->gpio = &gpio;
    this->invertState = invertState;

    this->gpio->isUsed = true;
    pinMode(this->gpio->pin, OUTPUT);
    this->setOff();
}

void Relay::setOff () {
    digitalWrite(this->gpio->pin, this->invertState ? HIGH : LOW);
}

void Relay::setOn () {
    digitalWrite(this->gpio->pin, this->invertState ? LOW : HIGH);
}

/**
 * true - on
 * false - off
*/
bool Relay::getState () {
    return digitalRead(this->gpio->pin) == (this->invertState ? 1 : 0) ? false : true;
}