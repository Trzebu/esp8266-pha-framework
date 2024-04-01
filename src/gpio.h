#include "vector"

class Gpio {
    public:
        Gpio(int pinNumber);
        int pin;
        bool isUsed = false;
};

class Pin {
    public:
        Pin();
        std::vector<Gpio> pins;
        void use(int pinNumber);
        bool isUsed(int pinNumber);
        Gpio& get(int pinNumber);
};