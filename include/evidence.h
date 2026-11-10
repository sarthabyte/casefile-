
#ifndef EVIDENCE_H
#define EVIDENCE_H

#include "structures.h"

void initializeEvidence(GameState *g);
int addEvidence(GameState *g, int evidenceID);
int hasEvidence(const GameState *g, int evidenceID);
void showEvidenceInventory(const GameState *g);
void freeEvidence(GameState *g);

#endif
