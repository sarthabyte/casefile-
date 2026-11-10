#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "suspects.h"

static Suspect suspects[MAX_SUSPECTS];
static int suspectCount = 0;

static void removeNewline(char *text)
{
    size_t length = strlen(text);

    while (length > 0 &&
           (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[length - 1] = '\0';
        length--;
    }
}

static int splitSuspectLine(char *line, char *fields[], int maxFields)
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

int loadSuspects(const char *filename)
{
    FILE *file;
    char line[512];
    int loadedCount = 0;

    if (filename == NULL) {
        return 0;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *fields[5];
        char *end;
        long id;
        Suspect suspect = {0};
        int fieldCount;
        int duplicate = 0;
        int i;

        removeNewline(line);

        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        fieldCount = splitSuspectLine(line, fields, 5);
        if (fieldCount != 5 || loadedCount >= MAX_SUSPECTS) {
            continue;
        }

        id = strtol(fields[0], &end, 10);
        if (*end != '\0' || id < 0 || id >= MAX_FLAGS) {
            continue;
        }

        suspect.id = (int)id;
        snprintf(suspect.name, sizeof(suspect.name), "%s", fields[1]);
        snprintf(suspect.occupation, sizeof(suspect.occupation), "%s", fields[2]);
        snprintf(suspect.alibi, sizeof(suspect.alibi), "%s", fields[3]);
        snprintf(suspect.dialogueFile, sizeof(suspect.dialogueFile), "%s", fields[4]);

        for (i = 0; i < loadedCount; i++) {
            if (suspects[i].id == suspect.id) {
                duplicate = 1;
                break;
            }
        }

        if (!duplicate && suspect.name[0] != '\0' && suspect.dialogueFile[0] != '\0') {
            suspects[loadedCount] = suspect;
            loadedCount++;
        }
    }

    fclose(file);
    suspectCount = loadedCount;
    return suspectCount;
}

const Suspect *getSuspectById(int id)
{
    int i;

    for (i = 0; i < suspectCount; i++) {
        if (suspects[i].id == id) {
            return &suspects[i];
        }
    }

    return NULL;
}

int getSuspectCount(void)
{
    return suspectCount;
}

const Suspect *getSuspectAt(int index)
{
    if (index < 0 || index >= suspectCount) {
        return NULL;
    }

    return &suspects[index];
}

void displaySuspects(void)
{
    int i;

    if (suspectCount == 0) {
        printf("No suspects loaded. Check data/suspects.txt.\n");
        return;
    }

    printf("\n========== SUSPECTS ==========\n");

    for (i = 0; i < suspectCount; i++) {
        printf("[%d] %s - %s\n", suspects[i].id,
               suspects[i].name, suspects[i].occupation);
        printf("    Alibi: %s\n", suspects[i].alibi);
    }
}