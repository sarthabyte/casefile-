#ifndef SUSPECTS_H
#define SUSPECTS_H
#include "structures.h"

#define MAX_SUSPECTS 16
int loadSuspects(const char *filename);
const Suspect *getSuspectById(int id);
int getSuspectCount(void);
const Suspect *getSuspectAt(int index);
void displaySuspects(void);

#endif
