#include <ESP8266WiFi.h>
#include "WifiManager.h"
#include "filesystem.h"

WifiManager::WifiManager (AsyncWebServer& server) {
    this->server = &server;
};

void WifiManager::init () {
    this->loadSettings();

    if (this->ssid == "") {
        Serial.println("Wifi configurations not found.");
        this->initAp();
        return;
    }

    WiFi.mode(WIFI_STA);
    localIP.fromString(this->ip.c_str());
    localGateway.fromString(this->gateway.c_str());
    IPAddress subnet(255, 255, 0, 0);

    if (!WiFi.config(localIP, localGateway, subnet)){
        Serial.println("STA Failed to configure");
        return;
    }

    WiFi.begin(this->ssid.c_str(), this->password.c_str());

    Serial.println("Connecting to WiFi...");
    delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect.");
        return;
    }

    Serial.println(WiFi.localIP());

    this->isInitialized = true;
}

String WifiManager::getIp () {
    return this->ip;
}

void WifiManager::loadSettings () {
    std::vector<std::string> wifi = getObject("wifi.txt");

    if (wifi.size() == 0) return;

    this->ssid = wifi[0].c_str();
    this->password = wifi[1].c_str();
    this->ip = wifi[2].c_str();
    this->gateway = wifi[3].c_str();
};

void WifiManager::saveSettings () {
    writeFile("wifi.txt", this->ssid + "," + this->password + "," + this->ip + "," + this->gateway);
};

void WifiManager::initAp () {
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("PHAESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    this->server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/wifimanager.html", "text/html");
    });

    this->server->on("/", HTTP_POST, [&](AsyncWebServerRequest *request) {
        int params = request->params();

        for (int i = 0; i < params; i++) {
            AsyncWebParameter* param = request->getParam(i);

            if (!param->isPost()) continue;

            if (param->name() == "ssid")
                this->ssid = param->value().c_str();
            else if (param->name() == "password")
                this->password = param->value().c_str();
            else if (param->name() == "ip")
                this->ip = param->value().c_str();
            else if (param->name() == "gateway")
                this->gateway = param->value().c_str();
        }

        if (this->ssid == "" || this->password == "" || this->ip == "" || this->gateway == "") {
            request->send(200, "text/plain", "Invalid data. Try again.");
            return;
        }

        this->saveSettings();

        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + this->ip);
        
        this->restartRequired = true;
    });
}