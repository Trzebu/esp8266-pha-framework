#include "Relay.h"

Relay::Relay (String name, Gpio& gpio) {
    this->name = name;
    this->gpio = &gpio;

    this->gpio->isUsed = true;
    pinMode(this->gpio->pin, OUTPUT);
    this->setOff();
}

void Relay::setOff () {
    digitalWrite(this->gpio->pin, HIGH);
}

void Relay::setOn () {
    digitalWrite(this->gpio->pin, LOW);
}

/**
 * true - on
 * false - off
*/
bool Relay::getState () {
    return digitalRead(this->gpio->pin) == 1 ? false : true;
}