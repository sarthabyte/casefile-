#include <stdio.h>
#include <stdlib.h>
#include "evidence.h"

/* Evidence is listed here once. Searching a room adds its clue to the game. */
static const Evidence evidenceCatalog[] = {
    {EVID_ACCESS_LOG, "Access card log", 0, 3, "Records show a card opened Flat 4B around 10:42 PM."},
    {EVID_CCTV, "CCTV footage", 0, 2, "A guard appears in the fourth-floor corridor, but the timestamp is unclear."},
    {EVID_CALL_RECORDING, "Call recording", 0, 1, "A scream is heard while Arjun is on the phone."},
    {EVID_SLIP_MARKS, "Stairwell marks", 0, 4, "Scuff marks suggest a struggle near the stairwell landing."},
    {EVID_GATE_LOG, "Gate visitor log", 0, 5, "The visitor log records Arjun entering at 11:05 PM, though the clock may be slow."},
    {EVID_COMPLAINT, "Complaint against Ravi", 0, 6, "A complaint was filed concerning Ravi's conduct."},
    {EVID_BROKEN_WATCH, "Broken wristwatch", 0, 7, "A stopped watch reads 10:48 PM; it may have stopped before or during the incident."},
    {EVID_WET_FOOTPRINT, "Wet footprints", 0, 8, "Wet shoeprints cross the service corridor and do not clearly identify their owner."},
    {EVID_TORN_NOTE, "Torn note", 0, 9, "A torn note mentions a meeting but has no name or complete time."},
    {EVID_MISSING_KEY, "Spare key register", 0, 10, "A spare key was signed out, but the return column is blank."},
    {EVID_CLEANING_RECEIPT, "Cleaning receipt", 0, 11, "A cleaning visit was booked for the evening, creating a possible timeline distraction."},
    {EVID_SECOND_PHONE, "Unregistered phone", 0, 12, "A second phone contains a deleted call entry with no saved contact name."}
};

#define EVIDENCE_COUNT (sizeof(evidenceCatalog) / sizeof(evidenceCatalog[0]))

void initializeEvidence(GameState *game)
{
    int locationID;

    if (game == NULL) {
        return;
    }

    game->evidence = NULL;
    game->flags[FLAG_INVESTIGATION_DONE] = 0;
    game->flags[FLAG_TIMELINE_SUBMITTED] = 0;
    game->flags[FLAG_ROOM_COUNT] = 0;

    /* Clear the visited flag for every room. */
    for (locationID = 1; locationID <= LOCATION_COUNT; locationID++) {
        game->flags[FLAG_ROOM_VISITED_BASE + locationID] = 0;
    }
}

int hasEvidence(const GameState *game, int evidenceID)
{
    const EvidenceNode *current;

    if (game == NULL) {
        return 0;
    }

    /* Walk through the linked list until the clue is found or the list ends. */
    current = game->evidence;
    while (current != NULL) {
        if (current->data.id == evidenceID) {
            return 1;
        }
        current = current->next;
    }

    return 0;
}

int addEvidence(GameState *game, int evidenceID)
{
    const Evidence *item = NULL;
    EvidenceNode *newNode;
    int i;

    if (game == NULL) {
        return 0;
    }

    if (hasEvidence(game, evidenceID)) {
        printf("This evidence has already been collected.\n");
        return 0;
    }

    /* Find the matching clue in the catalog. */
    for (i = 0; i < (int)EVIDENCE_COUNT; i++) {
        if (evidenceCatalog[i].id == evidenceID) {
            item = &evidenceCatalog[i];
            break;
        }
    }

    if (item == NULL) {
        printf("Invalid evidence ID.\n");
        return 0;
    }

    /* Make a new linked-list node to store the clue. */
    newNode = malloc(sizeof(EvidenceNode));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }

    newNode->data = *item;
    newNode->data.discovered = 1;
    newNode->next = game->evidence;
    game->evidence = newNode;

    if (FLAG_EVIDENCE_BASE + evidenceID < MAX_FLAGS) {
        game->flags[FLAG_EVIDENCE_BASE + evidenceID] = 1;
    }

    printf("\nEvidence collected: %s\n", item->name);
    printf("%s\n", item->description);
    return 1;
}

void showEvidenceInventory(const GameState *game)
{
    const EvidenceNode *current;

    if (game == NULL) {
        return;
    }

    printf("\n========== EVIDENCE INVENTORY ==========\n");
    if (game->evidence == NULL) {
        printf("No evidence collected yet.\n");
        return;
    }

    current = game->evidence;
    while (current != NULL) {
        printf("\nID: %d\n", current->data.id);
        printf("Name: %s\n", current->data.name);
        printf("Description: %s\n", current->data.description);
        current = current->next;
    }
}

void freeEvidence(GameState *game)
{
    EvidenceNode *current;

    if (game == NULL) {
        return;
    }

    /* Free every node before setting the list head to NULL. */
    current = game->evidence;
    while (current != NULL) {
        EvidenceNode *nextNode = current->next;
        free(current);
        current = nextNode;
    }

    game->evidence = NULL;
}
