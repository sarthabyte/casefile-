#ifndef FILES_H
#define FILES_H

#include "structures.h"

int saveGame(const GameState *game);
int loadGame(GameState *game);

int saveGameToFile(const GameState *game, const char *filename);
int loadGameFromFile(GameState *game, const char *filename);

#endif
