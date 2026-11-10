#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "locker.h"

static void sortCluesByRoom(LockerClue clues[LOCKER_CLUE_COUNT])
{
    int i, j;

    /* Bubble sort: the player reads digits by ascending room number. */
    for (i = 0; i < LOCKER_CLUE_COUNT - 1; i++) {
        for (j = 0; j < LOCKER_CLUE_COUNT - i - 1; j++) {
            if (clues[j].roomID > clues[j + 1].roomID) {
                LockerClue temporary = clues[j];
                clues[j] = clues[j + 1];
                clues[j + 1] = temporary;
            }
        }
    }
}

void initializeLockerPuzzle(GameState *game)
{
    char digits[LOCKER_CLUE_COUNT];
    int i;

    if (game == NULL) {
        return;
    }

    /* Seed once for this new game. clock() also varies rapid restarts. */
    srand((unsigned int)time(NULL) ^ (unsigned int)clock());

    /* A six-character code is used so a leading zero is preserved. */
    digits[0] = (char)('1' + rand() % 9);
    for (i = 1; i < LOCKER_CLUE_COUNT; i++) {
        digits[i] = (char)('0' + rand() % 10);
    }

    /* Shuffle the digits among the six clue rooms. */
    for (i = LOCKER_CLUE_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char temporary = digits[i];
        digits[i] = digits[j];
        digits[j] = temporary;
    }

    /* Keep the first digit nonzero so the sorted result is a six-digit number. */
    if (digits[0] == '0') {
        for (i = 1; i < LOCKER_CLUE_COUNT; i++) {
            if (digits[i] != '0') {
                char temporary = digits[0];
                digits[0] = digits[i];
                digits[i] = temporary;
                break;
            }
        }
    }

    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        game->lockerClues[i].roomID = i + 1;
        game->lockerClues[i].digit = digits[i];
        game->lockerClues[i].discovered = 0;
    }

    game->lockerFound = 0;
    game->lockerOpened = 0;
}

void revealLockerDigit(GameState *game, int roomID)
{
    int i;

    if (game == NULL) {
        return;
    }

    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        LockerClue *clue = &game->lockerClues[i];
        if (clue->roomID == roomID) {
            if (!clue->discovered) {
                clue->discovered = 1;
                printf("\nYou found a locker-code digit in this room: %c\n", clue->digit);
                printf("Record it with room number %d. The final order is ascending room number.\n",
                       clue->roomID);
            } else {
                printf("You have already recorded this room's locker-code digit.\n");
            }
            return;
        }
    }
}

void discoverLocker(GameState *game, int roomID)
{
    if (game == NULL || roomID != LOCKER_ROOM_ID) {
        return;
    }

    if (!game->lockerFound) {
        game->lockerFound = 1;
        printf("\nBehind the painting, you discover a concealed six-digit locker.\n");
        printf("The combination is not written on it. Find the six digits in the clue rooms.\n");
    }
}

void displayLockerClues(const GameState *game)
{
    LockerClue sorted[LOCKER_CLUE_COUNT];
    int i;

    if (game == NULL) {
        return;
    }

    memcpy(sorted, game->lockerClues, sizeof sorted);
    sortCluesByRoom(sorted);

    printf("\n========== LOCKER CODE CLUES ==========\n");
    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        if (sorted[i].discovered) {
            printf("Room %2d : %c\n", sorted[i].roomID, sorted[i].digit);
        } else {
            printf("Room %2d : ?\n", sorted[i].roomID);
        }
    }
    printf("Read the digits from the lowest room number to the highest.\n");
}

int useLocker(GameState *game)
{
    char entered[64];
    char correct[LOCKER_CLUE_COUNT + 1];
    LockerClue sorted[LOCKER_CLUE_COUNT];
    int i;

    if (game == NULL) {
        return 0;
    }

    if (!game->lockerFound) {
        printf("You have not discovered the hidden locker yet.\n");
        return 0;
    }

    if (game->lockerOpened) {
        printf("The locker is already open. Its sealed case documents have been recovered.\n");
        return 1;
    }

    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        if (!game->lockerClues[i].discovered) {
            printf("You have not found all six digits yet.\n");
            displayLockerClues(game);
            return 0;
        }
    }

    memcpy(sorted, game->lockerClues, sizeof sorted);
    sortCluesByRoom(sorted);
    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        correct[i] = sorted[i].digit;
    }
    correct[LOCKER_CLUE_COUNT] = '\0';

    displayLockerClues(game);
    printf("Enter the six-digit combination: ");
    if (fgets(entered, sizeof entered, stdin) == NULL) {
        return 0;
    }
    entered[strcspn(entered, "\r\n")] = '\0';

    if (strlen(entered) != LOCKER_CLUE_COUNT) {
        printf("The combination must contain exactly six digits.\n");
        return 0;
    }
    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        if (entered[i] < '0' || entered[i] > '9') {
            printf("The combination can contain digits only.\n");
            return 0;
        }
    }

    if (strcmp(entered, correct) != 0) {
        printf("The combination is incorrect. The locker remains locked.\n");
        return 0;
    }

    game->lockerOpened = 1;
    printf("\nCLICK. The locker opens.\n");
    printf("Inside is a sealed case-document envelope, preserved for the final case review.\n");
    return 1;
}
