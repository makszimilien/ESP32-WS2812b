#ifndef FILEHANDLING_H
#define FILEHANDLING_H

#include "SPIFFS.h"
#include <Arduino.h>

void initFS();
String readFileJson(fs::FS &fs, const char *path, const char *property);
void writeFileJson(fs::FS &fs, const char *path, const char *property,
                   const char *value);

#endif // FILEHANDLING_H