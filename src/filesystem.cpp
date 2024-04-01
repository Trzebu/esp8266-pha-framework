#include "filesystem.h"
#include "split.h"

void initFS () {
    if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
    } else {
        Serial.println("LittleFS mounted successfully");
    }
}

std::string readFile (const char * path) {
    Serial.printf("Reading file: %s\r\n", path);

    File file = LittleFS.open(path, "r");
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return String().c_str();
    }

    String fileContent;
    while (file.available()) {
        fileContent = file.readStringUntil('\n');
        break;
    }

    file.close();
    
    return fileContent.c_str();
} 

std::vector<std::string> getObject (const char * path) {
    std::string data = readFile(path);

    return split(data, ',');
}

std::vector<std::vector<std::string>> getObjectsArray (const char * path) {
    std::string data = readFile(path);
    std::vector<std::string> rows = split(data, '/');
    std::vector<std::vector<std::string>> parsedData;

    for (auto & row : rows) {
        parsedData.push_back(split(row, ','));
    }

    return parsedData;
}

void writeFile (const char * path, String message) {
    Serial.printf("Writing file: %s\r\n", path);

    File file = LittleFS.open(path, "w");
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }

    if (file.print(message)) {
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    
    file.close();
}