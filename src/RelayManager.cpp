#include "RelayManager.h"
#include "filesystem.h"


RelayManager::RelayManager (Pin& pin) {
    this->pin = &pin;
}

void RelayManager::init () {
    this->load();
}

void RelayManager::add (String name, int gpio, bool invertedState) {
    this->relays.push_back(
        Relay(name, this->pin->get(gpio), invertedState)
    );
    this->save();
}

void RelayManager::remove (String name) {
    for (unsigned int i = 0; i < this->relays.size(); i++) {
        if (this->relays[i].name == name) {
            this->relays[i].gpio->isUsed = false;
            this->relays.erase(this->relays.begin() + i);
        }
    }
    this->save();
}

bool RelayManager::exists (String name) {
    for (auto & relay : this->relays) {
        if (relay.name == name) 
            return true;
    }

    return false;
}

bool RelayManager::exists (int gpio) {
    for (auto & relay : this->relays) {
        if (relay.gpio->pin == gpio) 
            return true;
    }

    return false;
}

void RelayManager::setState (String name, String state) {
    for (auto & relay : this->relays) {
        if (relay.name == name) {
            String relayState = relay.getState() == true ? "on" : "off";
            if (relayState == state) return;
            if (state == "on")
                relay.setOn();
            else
                relay.setOff();
            break;
        }
    }
}

void RelayManager::save () {
    String data = "";

    for (auto relay = this->relays.begin(); relay != this->relays.end(); ++relay) {
        String invertState = relay->invertState ? "1" : "0";
        data = data + relay->name + "," + relay->gpio->pin + "," + invertState;

        if (std::next(relay) != this->relays.end())
            data = data + "/";
    }

    writeFile(this->DATA_FILE_NAME, data);
}

void RelayManager::load () {
    std::vector<std::vector<std::string>> relays = getObjectsArray(this->DATA_FILE_NAME);

    for (auto relay : relays) {
        bool invertState = relay[2] == "1" ? true : false;
        this->relays.push_back(
            Relay(relay[0].c_str(), this->pin->get(std::stoi(relay[1])), invertState)
        );
    }
}