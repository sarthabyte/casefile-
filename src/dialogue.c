#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dialogue.h"
#include "suspects.h"

static Dialogue dialogueLines[MAX_DIALOGUE_LINES];
static int dialogueCount = 0;

static void removeNewline(char *text)
{
    size_t length = strlen(text);

    while (length > 0 &&
           (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[length - 1] = '\0';
        length--;
    }
}

static int splitDialogueLine(char *line, char *fields[], int maxFields)
{
    int fieldCount = 1;
    char *current = line;

    fields[0] = line;

    while (*current != '\0' && fieldCount < maxFields) {
        if (*current == '|') {
            *current = '\0';
            fields[fieldCount] = current + 1;
            fieldCount++;
        }
        current++;
    }

    return fieldCount;
}

static int readDialogueFile(const char *path)
{
    FILE *file;
    char line[512];
    int addedCount = 0;

    file = fopen(path, "r");
    if (file == NULL) {
        printf("Warning: could not open dialogue file %s\n", path);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *fields[4];
        char *endID;
        char *endSpeaker;
        char *endRequired;
        long id;
        long speakerID;
        long requiredFlag;
        Dialogue dialogue = {0};
        int fieldCount;
        int duplicate = 0;
        int i;

        removeNewline(line);

        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        fieldCount = splitDialogueLine(line, fields, 4);
        if (fieldCount != 4 || dialogueCount >= MAX_DIALOGUE_LINES) {
            continue;
        }

        id = strtol(fields[0], &endID, 10);
        speakerID = strtol(fields[1], &endSpeaker, 10);
        requiredFlag = strtol(fields[2], &endRequired, 10);

        if (*endID != '\0' || *endSpeaker != '\0' || *endRequired != '\0') {
            continue;
        }

        if (id <= 0 || id >= FLAG_PROVEN_BASE ||
            speakerID < 0 || speakerID >= MAX_FLAGS ||
            requiredFlag < -1 || requiredFlag >= MAX_FLAGS ||
            fields[3][0] == '\0') {
            continue;
        }

        for (i = 0; i < dialogueCount; i++) {
            if (dialogueLines[i].id == (int)id) {
                duplicate = 1;
                break;
            }
        }

        if (duplicate) {
            printf("Warning: duplicate dialogue ID %ld ignored.\n", id);
            continue;
        }

        dialogue.id = (int)id;
        dialogue.speakerId = (int)speakerID;
        dialogue.requiredFlag = (int)requiredFlag;
        snprintf(dialogue.text, sizeof(dialogue.text), "%s", fields[3]);

        dialogueLines[dialogueCount] = dialogue;
        dialogueCount++;
        addedCount++;
    }

    fclose(file);
    return addedCount;
}

int loadDialogueForSuspect(const Suspect *suspect)
{
    char path[256];

    if (suspect == NULL || suspect->dialogueFile[0] == '\0') {
        return 0;
    }

    snprintf(path, sizeof(path), "data/%s", suspect->dialogueFile);
    return readDialogueFile(path);
}

int loadAllDialogue(void)
{
    int i;

    dialogueCount = 0;

    for (i = 0; i < getSuspectCount(); i++) {
        loadDialogueForSuspect(getSuspectAt(i));
    }

    return dialogueCount;
}

int getDialogueCount(void)
{
    return dialogueCount;
}

const Dialogue *getDialogueAt(int index)
{
    if (index < 0 || index >= dialogueCount) {
        return NULL;
    }

    return &dialogueLines[index];
}

void displayDialogueForSuspect(GameState *game, int suspectID)
{
    const Suspect *suspect;
    int shownCount = 0;
    int i;

    if (game == NULL) {
        printf("Invalid game state.\n");
        return;
    }

    suspect = getSuspectById(suspectID);
    if (suspect == NULL) {
        printf("Invalid suspect.\n");
        return;
    }

    printf("\n--- QUESTIONING: %s ---\n", suspect->name);
    printf("%s\n", suspect->alibi);

    for (i = 0; i < dialogueCount; i++) {
        const Dialogue *dialogue = &dialogueLines[i];

        if (dialogue->speakerId != suspectID) {
            continue;
        }

        if (dialogue->requiredFlag >= 0 && !game->flags[dialogue->requiredFlag]) {
            continue;
        }

        printf("Statement %d: %s\n", dialogue->id, dialogue->text);

        game->flags[dialogue->id] = 1;
        shownCount++;
    }

    if (shownCount == 0) {
        printf("No statements are available for this person.\n");
    }
}