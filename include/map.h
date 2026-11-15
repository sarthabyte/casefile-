#ifndef MAP_H
#define MAP_H

#include "structures.h"

void initializeMap(void);
void displayMap(void);
void displayConnectionTable(void);
void displayLocation(int locationID);
int moveToLocation(int currentID, int targetID);
Location *getLocation(int locationID);

#endif
