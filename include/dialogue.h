#ifndef DIALOGUE_H
#define DIALOGUE_H
#include "structures.h"

#define MAX_DIALOGUE_LINES 160
int loadDialogueForSuspect(const Suspect *suspect);
int loadAllDialogue(void);
int getDialogueCount(void);
const Dialogue *getDialogueAt(int index);
void displayDialogueForSuspect(GameState *game, int suspectId);

#endif
