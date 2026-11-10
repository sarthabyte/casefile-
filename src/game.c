#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "structures.h"
#include "game.h"
#include "map.h"
#include "evidence.h"
#include "investigation.h"
#include "suspects.h"
#include "dialogue.h"
#include "courtroom.h"
#include "files.h"
#include "screen.h"
#include "locker.h"



static int readMenuChoice(const char *prompt)
{
    char line[64];
    char *end;
    long value;

    printf("%s", prompt);
    if (fgets(line, sizeof line, stdin) == NULL)
        return -1;

    value = strtol(line, &end, 10);
    if (end == line || value < 0 || value > 100)
        return -1;

    while (*end == ' ' || *end == '\t')
        end++;

    if (*end != '\n' && *end != '\0')
        return -1;

    return (int)value;
}

/* Display one prologue line at a time, like a visual-novel dialogue box. */
static void showPrologueLine(const char *line)
{
    printf("\n%s\n", line);
    waitForEnter();
}

int startNewGame(void)
{
    printf("Starting game...\n");

#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif

    /* Remove the temporary startup message before showing the title. */
    clearScreen();
    printf("\n========================================\n");
    printf("                 CASEFILE                \n");
    printf("========================================\n");
    printf("\nPROLOGUE\n");
    printf("(Press Enter to advance each line.)\n");

    showPrologueLine("It was day just like another day until...");
    showPrologueLine("Welcome to the exciting world of POKEMON, trainers bond with their Pokemon and fight together to be the best Pokemon trainer in the world...");
    showPrologueLine("Aditya: AYO dude, turn that off. You are a 20-year-old; why are you still watching Pokemon?");
    showPrologueLine("Daksh: Shut the up. I couldn't complete it all this time.");
    showPrologueLine("Aditya: You Shut up and throw the trash out.");
    showPrologueLine("Daksh: Okay, going, man. Chill out.");
    showPrologueLine("AHHHHH! Dude, Dude! come out! COME OUT, I SAID!");
    showPrologueLine("Aditya: What are you rambling about...? AYO, WHAT DID YOU DO?");
    showPrologueLine("Daksh: I didn't do anything. I just called the elevator up, and it was already inside.");
    showPrologueLine("Aditya: We need to report this to the police...");

    clearScreen();
    return 1;
}

static int selectedCount(const GameState *game, int base, int maxId, int evidenceMode)
{
    int count = 0;
    for (int id = evidenceMode ? 1 : 0; id <= maxId; id++)
        if (game->flags[base + id]) count++;
    return count;
}

static void printTimelineSelections(const GameState *game)
{
    printf("\n========== CASE TIMELINE / EVIDENCE BOARD ==========\n");
    printf("Evidence is listed by ID, not proven chronological order. Clue times may be uncertain.\n\n");
    printf("Selected suspects:\n");
    int suspectShown = 0;
    for (int i = 0; i < getSuspectCount(); i++) {
        const Suspect *suspect = getSuspectAt(i);
        if (suspect && game->flags[FLAG_SELECTED_SUSPECT_BASE + suspect->id]) {
            printf("  [%d] %s - %s\n", suspect->id, suspect->name, suspect->alibi);
            suspectShown++;
        }
    }
    if (!suspectShown) printf("  (none selected)\n");

    printf("SELECTED EVIDENCE\n");
    int evidenceShown = 0;
    for (int id = 1; id <= EVID_SECOND_PHONE; id++) {
        if (!game->flags[FLAG_SELECTED_EVIDENCE_BASE + id]) continue;
        for (const EvidenceNode *n = game->evidence; n; n = n->next) {
            if (n->data.id == id) {
                printf("  [%d] %s — %s\n", id, n->data.name, n->data.description);
                evidenceShown++;
                break;
            }
        }
    }
    if (!evidenceShown) printf("  (none selected)\n");
    printf("===================================================\n");
}

static int showTimeline(GameState *game)
{
    if (!game) return 0;
    if (game->flags[FLAG_TIMELINE_SUBMITTED]) {
        printf("The timeline has already been submitted. Phase 1 is complete.\n");
        return 1;
    }

    for (;;) {
        clearScreen();
        printf("\n========== TIMELINE / PHASE 1 REVIEW ==========\n");
        printf("Collected evidence: ");
        int collected = 0;
        for (const EvidenceNode *n = game->evidence; n; n = n->next) collected++;
        printf("%d item(s)\n", collected);
        printf("Selected suspects: %d | Selected evidence: %d\n",
               selectedCount(game, FLAG_SELECTED_SUSPECT_BASE, MAX_SUSPECTS - 1, 0),
               selectedCount(game, FLAG_SELECTED_EVIDENCE_BASE, EVID_SECOND_PHONE, 1));
        printf("1. Select/deselect a suspect\n");
        printf("2. Select/deselect collected evidence\n");
        printf("3. Review timeline materials\n");
        printf("4. Submit timeline and end Phase 1\n");
        printf("0. Return to investigation\n");
        int choice = readMenuChoice("Choose: ");

        if (choice == 0) return 0;
        if (choice == 1) {
            clearScreen();
            displaySuspects();
            int id = readMenuChoice("Suspect ID to toggle (or 99 to cancel): ");
            const Suspect *suspect = getSuspectById(id);
            if (id == 99) continue;
            if (!suspect) { printf("Unknown suspect ID.\n"); continue; }
            int *selected = &game->flags[FLAG_SELECTED_SUSPECT_BASE + id];
            *selected = !*selected;
            printf("%s %s for the timeline.\n", suspect->name, *selected ? "selected" : "removed");
            waitForEnter();
        } else if (choice == 2) {
            clearScreen();
            showEvidenceInventory(game);
            int id = readMenuChoice("Evidence ID to toggle (or 99 to cancel): ");
            if (id == 99) continue;
            if (id < 1 || id > EVID_SECOND_PHONE || !hasEvidence(game, id)) {
                printf("You can only select evidence you have collected.\n"); continue;
            }
            int *selected = &game->flags[FLAG_SELECTED_EVIDENCE_BASE + id];
            *selected = !*selected;
            printf("Evidence %d %s for the timeline.\n", id, *selected ? "selected" : "removed");
            waitForEnter();
        } else if (choice == 3) {
            clearScreen();
            printTimelineSelections(game);
            waitForEnter();
        } else if (choice == 4) {
            int suspects = selectedCount(game, FLAG_SELECTED_SUSPECT_BASE, MAX_SUSPECTS - 1, 0);
            int evidence = selectedCount(game, FLAG_SELECTED_EVIDENCE_BASE, EVID_SECOND_PHONE, 1);
            if (!game->lockerOpened) {
                printf("Find and open the hidden locker before submitting the timeline.\n");
                waitForEnter();
                continue;
            }
            if (suspects == 0 || evidence == 0) {
                printf("Select at least one suspect and one collected evidence item before submitting.\n");
                waitForEnter();
                continue;
            }
            clearScreen();
            printTimelineSelections(game);
            int confirm = readMenuChoice("Submit these materials and end Phase 1? (1=yes, 0=no): ");
            if (confirm != 1) continue;
            game->flags[FLAG_TIMELINE_SUBMITTED] = 1;
            game->phase = PHASE_DEFENCE;
            printf("\nTIMELINE SUBMITTED. Phase 1 is complete. The selected suspects and evidence are now locked for Phase 2.\n");
            waitForEnter();
            return 1;
        } else {
            printf("Invalid option.\n");
            waitForEnter();
        }
    }
}

static void showGuidelines(void)
{
    printf("\n========== GUIDELINES =========="
           "\n1. There are 12 locations, but you may investigate a maximum of 6 distinct rooms."
           "\n2. The six essential evidence clues and six locker digits are in Rooms 1-6. Searching Rooms 7-12 uses an investigation opportunity but yields no essential evidence."
           "\n3. The hidden locker is behind the painting in the Manager's Office (Room 6)."
           "\n4. Record each digit with its room number, then arrange digits by ascending room number."
           "\n5. The six-digit combination is randomized for each new game and remains unchanged in a saved game."
           "\n6. Use the map to move between connected locations; movement does not consume an investigation."
           "\n7. Question suspects and use only evidence that contradicts their statements in court."
           "\n8. Run the game from the main project folder so the data files can be found.\n");
}

static void questionSuspect(GameState *game)
{
    int id;

    clearScreen();
    displaySuspects();
    id = readMenuChoice("Enter suspect ID (0-3), or 9 to return: ");

    if (id == 9)
        return;

    if (getSuspectById(id) == NULL) {
        printf("That suspect ID does not exist.\n");
        waitForEnter();
        return;
    }

    displayDialogueForSuspect(game, id);
    waitForEnter();
}


static void runGameSession(int loadExisting)
{
    GameState game;
    int choice;
    int loaded = 0;

    memset(&game, 0, sizeof game);
    game.phase = PHASE_DETECTIVE;
    game.playerLocation = 1;
    game.outcome = OUTCOME_NONE;

    initializeMap();

    if (loadSuspects("data/suspects.txt") <= 0) {
        printf("Could not load data/suspects.txt.\n");
        printf("Please run the game from the main project folder.\n");
        return;
    }

    if (loadAllDialogue() <= 0)
        printf("Warning: no dialogue lines were loaded. Check data/dialogue/*.txt.\n");

    if (loadExisting) {
        loaded = loadGame(&game);
        if (!loaded) {
            printf("Could not load savegame.dat. Starting a new case instead.\n");
            initializeEvidence(&game);
            initializeLockerPuzzle(&game);
            game.phase = PHASE_DETECTIVE;
            game.playerLocation = 1;
            game.outcome = OUTCOME_NONE;
        }
    } else {
        initializeEvidence(&game);
        initializeLockerPuzzle(&game);
    }

    printf("\nCASEFILE: Meena has been found dead in Flat 4B.\n");
    printf("Investigate up to six rooms, collect the six essential clues, find the hidden locker, and prepare your timeline.\n");

    waitForEnter();
    while (game.phase != PHASE_ENDED) {
        clearScreen();
        if (game.phase == PHASE_DETECTIVE) {
            printf("\n========== PHASE 1: DETECTIVE PHASE ==========\n");
            printf("1. Start Investigation\n2. Evidence / Inventory\n3. Timeline (select suspects/evidence; submit to end Phase 1)\n");
            printf("4. Suspects / Question a Suspect\n5. Guidelines\n6. Save Game\n7. Exit to Main Menu\n");
            choice = readMenuChoice("Enter option: ");
            clearScreen();
            switch (choice) {
                case 1: detectiveMode(&game); break;
                case 2: showEvidenceInventory(&game); waitForEnter(); break;
                case 3: (void)showTimeline(&game); break;
                case 4: questionSuspect(&game); break;
                case 5: showGuidelines(); waitForEnter(); break;
                case 6:
                    if (saveGame(&game)) printf("Game saved successfully.\n");
                    else printf("Game could not be saved.\n");
                    waitForEnter();
                    break;
                case 7: freeEvidence(&game); return;
                default: printf("Please enter a valid option from 1 to 7.\n"); waitForEnter(); break;
            }
        } else if (game.phase == PHASE_DEFENCE) {
            printf("\n========== PHASE 2: COURTROOM PHASE ==========\n");
            printf("1. Enter courtroom\n2. Save Game\n3. Exit to Main Menu\n");
            choice = readMenuChoice("Enter option: ");
            clearScreen();
            if (choice == 1) (void)runCourtroom(&game);
            else if (choice == 2) {
                if (saveGame(&game)) printf("Game saved successfully.\n");
                else printf("Game could not be saved.\n");
                waitForEnter();
            } else if (choice == 3) { freeEvidence(&game); return; }
            else { printf("Please enter a valid option from 1 to 3.\n"); waitForEnter(); }
        }
    }

    freeEvidence(&game);
    printf("\nCase closed. Returning to the main menu.\n");
}

void rungame(void)
{
    if (startNewGame())
        runGameSession(0);
}

void runLoadedGame(void)
{
    runGameSession(1);
}