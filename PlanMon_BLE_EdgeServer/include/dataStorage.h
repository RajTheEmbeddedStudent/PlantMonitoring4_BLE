#ifndef DATASTORAGE_H
#define DATASTORAGE_H

//External libraries
#include "FS.h"
#include "SD.h"
#include <SPI.h>
//Dependency libraries


//Naming for the slave data
extern String nameSlave;
extern char FileName[50];

//Function declarations
void dataStorageInit();
void logSDCard(char *);
String getLastRowFirstColumn(const char * path);




#endif