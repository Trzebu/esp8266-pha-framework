#include "Arduino.h"
#include "gpio.h"

class Relay {

    public:
        Relay(String name, Gpio& gpio, bool invertState);
        void setOff();
        void setOn();
        bool getState();
        String name;
        Gpio* gpio;
        bool invertState;
};