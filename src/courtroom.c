#include <stdio.h>
#include <stdlib.h>
#include "structures.h"
#include "courtroom.h"
#include "screen.h"

extern int hasEvidence(const GameState *g, int evidenceId);

extern const Suspect *getSuspectById(int id);

typedef struct {
    int id;
    int suspectId;
    const char *text;
    int contradictedBy;
} Testimony;

static const Testimony testimony[] = {
    { 1, 0, "Ravi never left the lobby all night.",                 EVID_ACCESS_LOG },
    { 2, 0, "The lobby camera shows him at his desk the whole time.", EVID_CCTV },
    { 3, 0, "Arjun pushed Ravi off the fourth floor.",              EVID_SLIP_MARKS },
    { 4, 0, "Arjun was at the gate before 10:30, already angry.",   EVID_GATE_LOG },
    { 5, 0, "No complaint was ever filed against Ravi.",            EVID_COMPLAINT },
    { 6, 1, "I heard her scream on the phone and ran over.",        EVID_NONE },
    { 7, 1, "I fought him off. He slipped by himself.",             EVID_NONE },
    { 8, 1, "I threw the knife away because I panicked.",           EVID_NONE },
    { 9, 2, "I heard shouting around 11 PM.",                       EVID_NONE },
    {10, 2, "I saw the guard on the stairs near our floor.",        EVID_NONE },
    {11, 2, "I think the boyfriend was inside before she screamed.", EVID_CALL_RECORDING },
};
#define TESTIMONY_COUNT ((int)(sizeof(testimony) / sizeof(testimony[0])))


static const Testimony *findTestimony(int id) {
    for (int i = 0; i < TESTIMONY_COUNT; i++)
        if (testimony[i].id == id) return &testimony[i];
    return NULL;
}


static int isHeard(const GameState *g, const Testimony *t) {
    return g->flags[t->id];
}
static int isProven(const GameState *g, const Testimony *t) {
    return g->flags[FLAG_PROVEN_BASE + t->id];
}

static int readInt(const char *prompt) {
    char buf[32];
    printf("%s", prompt);
    if (!fgets(buf, sizeof buf, stdin)) return 0;
    return atoi(buf);
}


void displayEvidence(const GameState *g) {
    printf("\n--- COLLECTED EVIDENCE ---\n");
    if (!g->evidence) { printf("(none)\n"); return; }
    for (const EvidenceNode *n = g->evidence; n; n = n->next) {
        if (g->flags[FLAG_TIMELINE_SUBMITTED] &&
            !g->flags[FLAG_SELECTED_EVIDENCE_BASE + n->data.id]) continue;
        printf("[%d] %s - %s\n", n->data.id, n->data.name, n->data.description);
    }
}


void displayWitnessStatements(const GameState *g) {
    int shown = 0;
    printf("\n--- WITNESS STATEMENTS ---\n");
    for (int i = 0; i < TESTIMONY_COUNT; i++) {
        const Testimony *t = &testimony[i];
        if (!isHeard(g, t)) continue;
        if (g->flags[FLAG_TIMELINE_SUBMITTED] && !g->flags[FLAG_SELECTED_SUSPECT_BASE + t->suspectId]) continue;
        const Suspect *s = getSuspectById(t->suspectId);
        printf("[%d] %s (%s): \"%s\"%s\n", t->id,
               s ? s->name : "Unknown", s ? s->occupation : "",
               t->text, isProven(g, t) ? "  ** CONTRADICTED **" : "");
        shown++;
    }
    if (!shown) printf("(no statements heard - investigate more)\n");
}


int challengeTestimony(GameState *g, int statementId, int evidenceId) {
    const Testimony *t = findTestimony(statementId);
    if (!t || !isHeard(g, t) || isProven(g, t))
        return CHALLENGE_INVALID;
    if (!hasEvidence(g, evidenceId))
        return CHALLENGE_INVALID;
    if (g->flags[FLAG_TIMELINE_SUBMITTED] &&
        (evidenceId < 1 || evidenceId > EVID_SECOND_PHONE ||
         !g->flags[FLAG_SELECTED_EVIDENCE_BASE + evidenceId]))
        return CHALLENGE_INVALID;

    if (t->contradictedBy != EVID_NONE && t->contradictedBy == evidenceId) {
        g->flags[FLAG_PROVEN_BASE + t->id] = 1;
        return CHALLENGE_PROVEN;
    }
    g->flags[FLAG_COURT_MISTAKES]++;
    return CHALLENGE_WRONG;
}


int assessCaseStrength(const GameState *g) {
    int lies = 0, proven = 0;
    for (int i = 0; i < TESTIMONY_COUNT; i++) {
        if (testimony[i].contradictedBy == EVID_NONE) continue;
        lies++;
        if (isProven(g, &testimony[i])) proven++;
    }
    return lies ? (proven * 100) / lies : 0;
}


Outcome determineVerdict(GameState *g) {
    int strength = assessCaseStrength(g);
    if (g->flags[FLAG_COURT_MISTAKES] >= MAX_MISTAKES || strength < WIN_STRENGTH)
        g->outcome = OUTCOME_GUILTY;
    else
        g->outcome = OUTCOME_INNOCENT;
    g->phase = PHASE_ENDED;
    return g->outcome;
}

static void printVerdict(const GameState *g) {
    printf("\n=========== VERDICT ===========\n");
    if (g->outcome == OUTCOME_INNOCENT)
        printf("NOT GUILTY. Arjun is cleared, and Vikram's cover-up is exposed by the contradictions.\n");
    else
        printf("GUILTY. The defence failed to expose Vikram's lies beyond reasonable doubt.\n");
    printf("Case strength: %d%%\n", assessCaseStrength(g));
}


int runCourtroom(GameState *g) {
    g->phase = PHASE_DEFENCE;
    for (;;) {
        clearScreen();
        printf("\n===== COURTROOM ===== Strength %d%% | Mistakes %d/%d\n"
               "1.Review evidence  2.Witness statements  3.Challenge testimony\n"
               "4.Final argument   0.Return to investigation\n",
               assessCaseStrength(g), g->flags[FLAG_COURT_MISTAKES], MAX_MISTAKES);
        int c = readInt("> ");
        clearScreen();

        if (c == 1) {
            displayEvidence(g);
            waitForEnter();
        } else if (c == 2) {
            displayWitnessStatements(g);
            waitForEnter();
        } else if (c == 3) {
            displayWitnessStatements(g);
            int sid = readInt("Statement ID to challenge: ");
            int eid = readInt("Evidence ID to present: ");
            int r = challengeTestimony(g, sid, eid);
            if (r == CHALLENGE_PROVEN)
                printf("OBJECTION SUSTAINED! The statement is contradicted.\n");
            else if (r == CHALLENGE_WRONG)
                printf("OBJECTION OVERRULED. That does not contradict it.\n");
            else
                printf("Invalid: statement not heard, evidence not owned, or already proven.\n");
            if (g->flags[FLAG_COURT_MISTAKES] >= MAX_MISTAKES) {
                printf("Too many failed objections!\n");
                determineVerdict(g); printVerdict(g);
                waitForEnter();
                return COURT_VERDICT;
            }
            waitForEnter();
        } else if (c == 4) {
            determineVerdict(g); printVerdict(g);
            waitForEnter();
            return COURT_VERDICT;
        } else if (c == 0) {
            if (ALLOW_RETURN && !g->flags[FLAG_TIMELINE_SUBMITTED]) { g->phase = PHASE_DETECTIVE; return COURT_RETURN; }
            printf("Phase 1 has ended; you cannot return to the investigation.\n");
            waitForEnter();
        } else {
            printf("Invalid option.\n");
            waitForEnter();
        }
    }
}