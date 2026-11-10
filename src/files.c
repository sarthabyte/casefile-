#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
#include "files.h"

#define DEFAULT_SAVE_FILE "savegame.dat"
#define SAVE_VERSION 3
#define MAX_SAVED_EVIDENCE 12

static const unsigned char SAVE_MAGIC[8] = {'L','A','S','T','A','L','I','B'};

static int writeInt(FILE *fp, int value)
{
    return fwrite(&value, sizeof value, 1, fp) == 1;
}

static int readInt(FILE *fp, int *value)
{
    return fread(value, sizeof *value, 1, fp) == 1;
}

static void freeEvidenceList(EvidenceNode *node)
{
    while (node != NULL) {
        EvidenceNode *next = node->next;
        free(node);
        node = next;
    }
}

static int validGameState(const GameState *game)
{
    if (game == NULL)
        return 0;
    if (game->phase < PHASE_DETECTIVE || game->phase > PHASE_ENDED)
        return 0;
    if (game->playerLocation < 1 || game->playerLocation > LOCATION_COUNT)
        return 0;
    if (game->outcome < OUTCOME_NONE || game->outcome > OUTCOME_GUILTY)
        return 0;
    if (game->lockerFound < 0 || game->lockerFound > 1 ||
        game->lockerOpened < 0 || game->lockerOpened > 1 ||
        (game->lockerOpened && !game->lockerFound))
        return 0;
    {
        int seenRooms[LOCATION_COUNT + 1] = {0};
        int i;
        for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
            const LockerClue *clue = &game->lockerClues[i];
            if (clue->roomID < 1 || clue->roomID > LOCATION_COUNT ||
                seenRooms[clue->roomID] || clue->digit < '0' || clue->digit > '9' ||
                (clue->discovered != 0 && clue->discovered != 1))
                return 0;
            seenRooms[clue->roomID] = 1;
        }
    }
    return 1;
}

static int writeEvidence(FILE *fp, const Evidence *e)
{
    return writeInt(fp, e->id) &&
           fwrite(e->name, sizeof e->name, 1, fp) == 1 &&
           writeInt(fp, e->discovered) &&
           writeInt(fp, e->locationId) &&
           fwrite(e->description, sizeof e->description, 1, fp) == 1;
}

static int readEvidence(FILE *fp, Evidence *e)
{
    return readInt(fp, &e->id) &&
           fread(e->name, sizeof e->name, 1, fp) == 1 &&
           readInt(fp, &e->discovered) &&
           readInt(fp, &e->locationId) &&
           fread(e->description, sizeof e->description, 1, fp) == 1;
}

static int validEvidence(const Evidence *e)
{
    if (e->id < EVID_ACCESS_LOG || e->id > EVID_SECOND_PHONE)
        return 0;
    if (e->discovered != 0 && e->discovered != 1)
        return 0;
    if (e->locationId < 1 || e->locationId > LOCATION_COUNT)
        return 0;
    if (memchr(e->name, '\0', sizeof e->name) == NULL ||
        memchr(e->description, '\0', sizeof e->description) == NULL)
        return 0;
    return 1;
}

int saveGameToFile(const GameState *game, const char *filename)
{
    FILE *fp;
    const EvidenceNode *node;
    int count = 0;
    int seen[EVID_SECOND_PHONE + 1] = {0};

    if (!validGameState(game) || filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Save failed: invalid game state or filename.\n");
        return 0;
    }

    for (node = game->evidence; node != NULL; node = node->next) {
        if (count >= MAX_SAVED_EVIDENCE || !validEvidence(&node->data) ||
            seen[node->data.id]) {
            fprintf(stderr, "Save failed: evidence list is invalid.\n");
            return 0;
        }
        seen[node->data.id] = 1;
        count++;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Save failed: could not open save file");
        return 0;
    }

    int ok = fwrite(SAVE_MAGIC, sizeof SAVE_MAGIC, 1, fp) == 1 &&
             writeInt(fp, SAVE_VERSION) &&
             writeInt(fp, (int)game->phase) &&
             writeInt(fp, game->playerLocation) &&
             writeInt(fp, (int)game->outcome) &&
             writeInt(fp, game->lockerFound) &&
             writeInt(fp, game->lockerOpened);

    for (int i = 0; ok && i < LOCKER_CLUE_COUNT; i++) {
        ok = writeInt(fp, game->lockerClues[i].roomID) &&
             writeInt(fp, (unsigned char)game->lockerClues[i].digit) &&
             writeInt(fp, game->lockerClues[i].discovered);
    }

    for (int i = 0; ok && i < MAX_FLAGS; i++)
        ok = writeInt(fp, game->flags[i]);

    if (ok)
        ok = writeInt(fp, count);

    for (node = game->evidence; ok && node != NULL; node = node->next)
        ok = writeEvidence(fp, &node->data);

    if (fclose(fp) != 0)
        ok = 0;

    if (!ok) {
        fprintf(stderr, "Save failed: an error occurred while writing the file.\n");
        return 0;
    }

    printf("Game saved to %s.\n", filename);
    return 1;
}

int loadGameFromFile(GameState *game, const char *filename)
{
    FILE *fp;
    unsigned char magic[sizeof SAVE_MAGIC];
    int version, phase, location, outcome, count;
    int lockerFound, lockerOpened;
    GameState loaded;
    int seen[EVID_SECOND_PHONE + 1] = {0};

    if (game == NULL || filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Load failed: invalid game state or filename.\n");
        return 0;
    }

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Load failed: could not open save file");
        return 0;
    }

    memset(&loaded, 0, sizeof loaded);
    loaded.evidence = NULL;

    int ok = fread(magic, sizeof magic, 1, fp) == 1 &&
             memcmp(magic, SAVE_MAGIC, sizeof magic) == 0 &&
             readInt(fp, &version) && version == SAVE_VERSION &&
             readInt(fp, &phase) &&
             readInt(fp, &location) &&
             readInt(fp, &outcome) &&
             readInt(fp, &lockerFound) &&
             readInt(fp, &lockerOpened);

    if (ok) {
        loaded.phase = (Phase)phase;
        loaded.playerLocation = location;
        loaded.outcome = (Outcome)outcome;
        loaded.lockerFound = lockerFound;
        loaded.lockerOpened = lockerOpened;
        ok = lockerFound >= 0 && lockerFound <= 1 &&
             lockerOpened >= 0 && lockerOpened <= 1 &&
             (!lockerOpened || lockerFound);
    }

    for (int i = 0; ok && i < LOCKER_CLUE_COUNT; i++) {
        int digit;
        ok = readInt(fp, &loaded.lockerClues[i].roomID) &&
             readInt(fp, &digit) && digit >= '0' && digit <= '9' &&
             readInt(fp, &loaded.lockerClues[i].discovered);
        if (ok) {
            loaded.lockerClues[i].digit = (char)digit;
            if (loaded.lockerClues[i].discovered != 0 &&
                loaded.lockerClues[i].discovered != 1)
                ok = 0;
        }
    }
    if (ok)
        ok = validGameState(&loaded);

    for (int i = 0; ok && i < MAX_FLAGS; i++)
        ok = readInt(fp, &loaded.flags[i]);

    if (ok)
        ok = readInt(fp, &count) && count >= 0 && count <= MAX_SAVED_EVIDENCE;

    EvidenceNode **tail = &loaded.evidence;
    for (int i = 0; ok && i < count; i++) {
        EvidenceNode *node = (EvidenceNode *)calloc(1, sizeof *node);
        if (node == NULL) {
            ok = 0;
            break;
        }
        if (!readEvidence(fp, &node->data) || !validEvidence(&node->data) ||
            seen[node->data.id]) {
            free(node);
            ok = 0;
            break;
        }
        seen[node->data.id] = 1;
        node->next = NULL;
        *tail = node;
        tail = &node->next;
    }

    if (ok) {
        int extra = fgetc(fp);
        if (extra != EOF || ferror(fp))
            ok = 0;
    }
    fclose(fp);

    if (!ok) {
        freeEvidenceList(loaded.evidence);
        fprintf(stderr, "Load failed: save file is invalid, incomplete, or incompatible.\n");
        return 0;
    }

    freeEvidenceList(game->evidence);
    *game = loaded;
    printf("Game loaded from %s.\n", filename);
    return 1;
}

int saveGame(const GameState *game)
{
    return saveGameToFile(game, DEFAULT_SAVE_FILE);
}

int loadGame(GameState *game)
{
    return loadGameFromFile(game, DEFAULT_SAVE_FILE);
}