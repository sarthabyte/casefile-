#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structures.h"
#include "courtroom.h"

int hasEvidence(const GameState *g, int evidenceId) {
    for (const EvidenceNode *n = g->evidence; n; n = n->next)
        if (n->data.id == evidenceId) return 1;
    return 0;
}

static const Suspect fakeSuspects[3] = {
    {0, "Vikram", "Night Supervisor", "In the back office", ""},
    {1, "Arjun",  "Boyfriend",        "On a call, then ran over", ""},
    {2, "Meena",  "Neighbor",         "At home in 4C", ""},
};
const Suspect *getSuspectById(int id) {
    return (id >= 0 && id < 3) ? &fakeSuspects[id] : NULL;
}

static void addEvidence(GameState *g, int id, const char *name, const char *desc) {
    EvidenceNode *n = calloc(1, sizeof *n);
    n->data.id = id; n->data.discovered = 1;
    strcpy(n->data.name, name); strcpy(n->data.description, desc);
    n->next = g->evidence; g->evidence = n;
}

int main(void) {
    GameState g = {0};
    addEvidence(&g, EVID_ACCESS_LOG, "Access card log", "Ravi entered flat 4B at 10:42");
    addEvidence(&g, EVID_CCTV, "CCTV footage", "Guard on the 4th floor corridor");
    addEvidence(&g, EVID_CALL_RECORDING, "Call recording", "Scream heard while Arjun was on the phone");
    addEvidence(&g, EVID_SLIP_MARKS, "Slip marks", "Wet floor, accidental fall");
    addEvidence(&g, EVID_GATE_LOG, "Gate visitor log", "Arjun entered at 11:05");
    for (int i = 1; i <= 11; i++) g.flags[i] = 1;
    g.phase = PHASE_DEFENCE;

    int r = runCourtroom(&g);
    printf("\n[test] runCourtroom returned %d, outcome=%d\n", r, g.outcome);
    return 0;
}