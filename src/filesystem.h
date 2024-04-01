#include "LittleFS.h"

void initFS();

void writeFile(const char * path, String message);

std::string readFile (const char * path);

std::vector<std::string> getObject (const char * path);

std::vector<std::vector<std::string>> getObjectsArray (const char * path);