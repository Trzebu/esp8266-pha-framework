#include <ESPAsyncWebServer.h>
#include "Relay.h"

class RelayManager {

    public:
        RelayManager(Pin& pin);
        void init();
        void add(String name, int gpio);
        void remove(String name);
        bool exists(String name);
        bool exists(int gpio);
        void setState(String name, String state);
        std::vector<Relay> relays;
    private:
        Pin* pin;
        const char * DATA_FILE_NAME = "relays.txt";
        void save();
        void load();

};