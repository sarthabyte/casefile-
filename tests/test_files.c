#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "files.h"
#include "evidence.h"
#include "locker.h"

static void test_save_load_game(void)
{
    GameState original = {0};
    GameState restored = {0};

    original.phase = PHASE_DEFENCE;
    original.playerLocation = 3;
    original.outcome = OUTCOME_NONE;
    original.flags[1] = 1;
    original.flags[FLAG_PROVEN_BASE + 1] = 1;
    original.flags[FLAG_COURT_MISTAKES] = 2;

    initializeEvidence(&original);
    initializeLockerPuzzle(&original);
    original.lockerClues[0].discovered = 1;
    original.lockerFound = 1;
    original.flags[FLAG_INVESTIGATION_DONE] = 1;

    assert(addEvidence(&original, EVID_ACCESS_LOG));
    assert(addEvidence(&original, EVID_CCTV));

    assert(saveGameToFile(&original, "test_save.dat"));
    assert(loadGameFromFile(&restored, "test_save.dat"));

    assert(restored.phase == original.phase);
    assert(restored.playerLocation == original.playerLocation);
    assert(restored.outcome == original.outcome);
    assert(restored.flags[1] == 1);
    assert(restored.flags[FLAG_PROVEN_BASE + 1] == 1);
    assert(restored.flags[FLAG_COURT_MISTAKES] == 2);
    assert(restored.flags[FLAG_INVESTIGATION_DONE] == 1);
    assert(restored.lockerFound == original.lockerFound);
    assert(restored.lockerOpened == original.lockerOpened);
    for (int i = 0; i < LOCKER_CLUE_COUNT; i++) {
        assert(restored.lockerClues[i].roomID == original.lockerClues[i].roomID);
        assert(restored.lockerClues[i].digit == original.lockerClues[i].digit);
        assert(restored.lockerClues[i].discovered == original.lockerClues[i].discovered);
    }

    assert(hasEvidence(&restored, EVID_ACCESS_LOG));
    assert(hasEvidence(&restored, EVID_CCTV));

    freeEvidence(&original);
    freeEvidence(&restored);
    remove("test_save.dat");

    puts("PASS: Save/load game with evidence.");
}

static void test_empty_evidence(void)
{
    GameState original = {0};
    GameState restored = {0};

    original.phase = PHASE_DEFENCE;
    original.playerLocation = 1;
    original.outcome = OUTCOME_NONE;

    initializeEvidence(&original);
    initializeLockerPuzzle(&original);

    assert(saveGameToFile(&original, "test_empty.dat"));
    assert(loadGameFromFile(&restored, "test_empty.dat"));

    assert(restored.phase == original.phase);
    assert(restored.playerLocation == original.playerLocation);
    assert(!hasEvidence(&restored, EVID_ACCESS_LOG));
    assert(!hasEvidence(&restored, EVID_CCTV));

    freeEvidence(&original);
    freeEvidence(&restored);
    remove("test_empty.dat");

    puts("PASS: Save/load game with no evidence.");
}

static void test_missing_file(void)
{
    GameState restored = {0};

    remove("file_that_does_not_exist.dat");

    assert(!loadGameFromFile(
        &restored, "file_that_does_not_exist.dat"));

    puts("PASS: Missing file rejected.");
}

static void test_empty_file(void)
{
    GameState restored = {0};
    FILE *file = fopen("test_corrupt.dat", "wb");

    assert(file != NULL);
    fclose(file);

    assert(!loadGameFromFile(&restored, "test_corrupt.dat"));

    remove("test_corrupt.dat");

    puts("PASS: Empty file rejected.");
}

static void test_corrupt_file(void)
{
    GameState restored = {0};
    FILE *file = fopen("test_corrupt.dat", "wb");

    assert(file != NULL);
    fputs("THIS IS NOT A VALID SAVE FILE", file);
    fclose(file);

    assert(!loadGameFromFile(&restored, "test_corrupt.dat"));

    remove("test_corrupt.dat");

    puts("PASS: Corrupted file rejected.");
}

int main(void)
{
    test_save_load_game();
    test_empty_evidence();
    test_missing_file();
    test_empty_file();
    test_corrupt_file();

    puts("\nAll save/load tests passed!");
    return 0;
}