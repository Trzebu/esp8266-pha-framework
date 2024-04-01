#include "Arduino.h"
#include <ESPAsyncWebServer.h>

class WifiManager {
    public:
        WifiManager(AsyncWebServer& server);
        void init();
        String getIp();
        bool isInitialized = false;
        bool restartRequired = false;
    private:
        void loadSettings();
        void saveSettings();
        void initAp();
        String ssid = "";
        String password = "";
        String ip = "";
        String gateway = "";
        AsyncWebServer* server;
        IPAddress localIP;
        IPAddress localGateway;
};