#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "locker.h"

static void writeCombinationInput(const char *combination)
{
    FILE *file = fopen("test_locker_input.txt", "w");
    assert(file != NULL);
    fprintf(file, "%s\n", combination);
    fclose(file);
    assert(freopen("test_locker_input.txt", "r", stdin) != NULL);
}

int main(void)
{
    GameState game = {0};
    char correct[LOCKER_CLUE_COUNT + 1];
    char incorrect[LOCKER_CLUE_COUNT + 1];
    int i;

    initializeLockerPuzzle(&game);
    assert(game.lockerFound == 0);
    assert(game.lockerOpened == 0);

    for (i = 0; i < LOCKER_CLUE_COUNT; i++) {
        assert(game.lockerClues[i].roomID == i + 1);
        assert(game.lockerClues[i].digit >= '0');
        assert(game.lockerClues[i].digit <= '9');
        assert(game.lockerClues[i].discovered == 0);
        game.lockerClues[i].discovered = 1;
        correct[i] = game.lockerClues[i].digit;
    }
    correct[LOCKER_CLUE_COUNT] = '\0';
    assert(correct[0] != '0');

    /* Reverse the records to verify that locker logic sorts by room ID. */
    for (i = 0; i < LOCKER_CLUE_COUNT / 2; i++) {
        LockerClue temporary = game.lockerClues[i];
        game.lockerClues[i] = game.lockerClues[LOCKER_CLUE_COUNT - 1 - i];
        game.lockerClues[LOCKER_CLUE_COUNT - 1 - i] = temporary;
    }

    discoverLocker(&game, LOCKER_ROOM_ID);
    assert(game.lockerFound == 1);

    strcpy(incorrect, correct);
    incorrect[0] = incorrect[0] == '9' ? '8' : (char)(incorrect[0] + 1);
    writeCombinationInput(incorrect);
    assert(useLocker(&game) == 0);
    assert(game.lockerOpened == 0);

    writeCombinationInput(correct);
    assert(useLocker(&game) == 1);
    assert(game.lockerOpened == 1);

    remove("test_locker_input.txt");
    puts("PASS: Locker generation, validation, and opening.");
    return 0;
}
