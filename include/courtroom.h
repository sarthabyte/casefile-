#ifndef COURTROOM_H
#define COURTROOM_H

#include "structures.h"

#define MAX_MISTAKES        3
#define WIN_STRENGTH        60
#define ALLOW_RETURN        1

#define COURT_VERDICT       1
#define COURT_RETURN        0

#define CHALLENGE_INVALID   0
#define CHALLENGE_PROVEN    1
#define CHALLENGE_WRONG     2


int  runCourtroom(GameState *g);

void    displayEvidence(const GameState *g);
void    displayWitnessStatements(const GameState *g);
int     challengeTestimony(GameState *g, int statementId, int evidenceId);
int     assessCaseStrength(const GameState *g);
Outcome determineVerdict(GameState *g);

#endif