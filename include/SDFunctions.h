#include <Arduino.h>
#include "SPI.h"
#include "SD.h"
#include "FS.h"


void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void printDirectory(File dir, int numTabs);
bool readFile(fs::FS &fs, const char * path, uint8_t* buffer, size_t sz);
bool writeFile(fs::FS &fs, const char * path, String& buffer);
void appendFile(fs::FS &fs, const char * path, const char * message);

void renameFile(fs::FS &fs, const char * path1, const char * path2);