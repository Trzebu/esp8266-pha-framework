#include <Arduino.h>
#include "filesystem.h"
#include "WifiManager.h"
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "filesystem.h"
#include "RelayManager.h"
#include "split.h"

const String FIRMWARE_VERSION = "0.2";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WifiManager wifiManager = WifiManager(server);
Pin pin = Pin();
RelayManager relayManager = RelayManager(pin);

void routes () {
    server.on("/pha_esp_compatybile", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"firmware_version\":\"" + FIRMWARE_VERSION + "\"}");
    });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/relay", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/relay.html", "text/html");
    });
    server.on("/relay/new", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();

        String name = "";
        int gpio = -1;
        bool inverted = false;

        for (int i = 0; i < params; i++) {
            AsyncWebParameter* param = request->getParam(i);

            if (!param->isPost()) continue;
            if (param->name() == "name")
                name = param->value().c_str();
            else if (param->name() == "gpio")
                gpio = param->value().toInt();
            else if (param->name() == "inverted")
                inverted = param->value().toInt() == 1 ? true : false;
        }
        
        if (name == "" || gpio == -1) {
            request->send(200, "application/json", "{\"error\":\"Invalid data.\",\"code\":0}");
            return;
        }

        String error = "";
        String errorCode = "";

        if (!std::all_of(name.begin(), name.end(), [](unsigned char ch) {
            return std::isalnum(ch) || ch == '_';
        })) {
            error = "You can use only alphanumeric characters.";
            errorCode = "1";
        } else if (relayManager.exists(name)) {
            error = "Relay with that name already exists.";
            errorCode = "2";
        } else if (pin.isUsed(gpio)) {
            error = "This pin is already used.";
            errorCode = "3";
        }

        if (error != "") {
            request->send(200, "application/json", "{\"error\":\"" + error + "\",\"code\":" + errorCode + "}");
            return;
        }
        
        relayManager.add(name, gpio, inverted);
        request->send(204, "text/html", "No content");
    });
    server.on("/get_relays", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "[";

        for (auto relay = relayManager.relays.begin(); relay != relayManager.relays.end(); ++relay) {
            String data = "{\"name\":\"" + relay->name + "\",\"gpio\":" + relay->gpio->pin + ",\"inverted\":" + (relay->invertState ? "1" : "0") + ",\"state\":\"" + relay->getState() + "\"}";

            if (std::next(relay) != relayManager.relays.end())
                data = data + ",";

            json = json + data;
        }

        json = json + "]";

        request->send(200, "application/json", json);
    });
    server.on("/relay_delete", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebParameter* param = request->getParam(0);
        
        if (param) {
            if (param->name() == "name") {
                String name = param->value().c_str();
                if (relayManager.exists(name)) {
                    relayManager.remove(name);
                }
            }
        }

        if (request->host() == wifiManager.getIp()) 
            request->redirect("/relay");
        else
            request->send(200, "application/json", "{\"success\":\"true\"}");
    });
    server.on("/set_relay_state", HTTP_GET, [](AsyncWebServerRequest *request) {
        String name;
        String state;

        for (unsigned int i = 0; i < request->params(); i++) {
            AsyncWebParameter* param = request->getParam(i);

            if (param->name() == "name")
                name = param->value().c_str();
            else if (param->name() == "state")
                state = param->value().c_str();
        }

        if (name == "" || state == "") {
            request->send(200, "application/json", "{\"error\":\"Invalid data.\"}");
            return;
        } else if (state != "on" && state != "off") {
            request->send(200, "application/json", "{\"error\":\"Invalid data.\"}");
            return;
        }

        relayManager.setState(name, state);

        if (request->host() == wifiManager.getIp()) 
            request->redirect("/relay");
        else
            request->send(200, "application/json", "{\"success\":\"true\"}");
    });
    server.on("/get_unused_gpio_pins", HTTP_GET, [](AsyncWebServerRequest *request) {
        std::vector<Gpio> unused;
        String json = "[";

        std::copy_if (pin.pins.begin(), pin.pins.end(), std::back_inserter(unused), [](Gpio i){
            return !i.isUsed;
        });

        for (auto gpio = unused.begin(); gpio != unused.end(); ++gpio) {
            String data = String(gpio->pin);

            if (std::next(gpio) != unused.end())
                data = data + ",";

            json = json + data;
        }

        json = json + "]";

        request->send(200, "application/json", json);
    });
}

void wsResponseError (AsyncWebSocketClient *client, String error) {
    client->text("{\"error\":\"" + error + "\"}");
}

void processRequest (AsyncWebSocketClient *client, String data) {
    std::vector<std::string> chunks = split(data.c_str(), ',');
    std::string path = chunks[0];

    if (path == "set_relay_state") {
        if (chunks.size() < 3)
            return wsResponseError(client, "No enough parameters.");
        String relayName = chunks[1].c_str();
        String state = chunks[2].c_str();
        relayManager.setState(relayName, state);
    }
}

String handleWebSocketMessage (void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
        if(info->opcode == WS_TEXT){
            for(size_t i=0; i < info->len; i++) {
            msg += (char) data[i];
            }
        } else {
            char buff[3];
            for(size_t i=0; i < info->len; i++) {
                sprintf(buff, "%02x", (uint8_t) data[i]);
                msg += buff ;
            }
        }
    }
    return msg;
}

void onEvent (AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            processRequest(client, handleWebSocketMessage(arg, data, len));
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void setup () {
    Serial.begin(115200);
    initFS();
    wifiManager.init();
    server.serveStatic("/", LittleFS, "/");

    if (wifiManager.isInitialized) {
        relayManager.init();
        routes();
    }

    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();   
}

void loop() {
    if (wifiManager.restartRequired) {
        delay(1000);
        ESP.restart();
    }

    ws.cleanupClients();
}