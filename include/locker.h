#ifndef LOCKER_H
#define LOCKER_H

#include "structures.h"

void initializeLockerPuzzle(GameState *game);
void revealLockerDigit(GameState *game, int roomID);
void discoverLocker(GameState *game, int roomID);
int useLocker(GameState *game);
void displayLockerClues(const GameState *game);

#endif
