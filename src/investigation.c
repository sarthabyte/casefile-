#include <stdio.h>
#include <stdlib.h>
#include "structures.h"
#include "map.h"
#include "evidence.h"
#include "investigation.h"
#include "screen.h"
#include "locker.h"

/* Read a whole line so letters such as "abc" are not mistaken for a number. */
static int readChoice(const char *prompt)
{
    char line[64];
    char *end;
    long value;

    printf("%s", prompt);
    if (fgets(line, sizeof(line), stdin) == NULL) {
        return -1;
    }

    value = strtol(line, &end, 10);
    if (end == line || value < 0 || value > 100) {
        return -1;
    }

    /* Allow spaces after the number, but no extra text. */
    while (*end == ' ' || *end == '\t') {
        end++;
    }
    if (*end != '\n' && *end != '\0') {
        return -1;
    }

    return (int)value;
}

int completeInvestigation(GameState *game)
{
    if (game == NULL) {
        return 0;
    }

    /* Phase 1 ends only when the player submits the timeline. */
    return game->flags[FLAG_TIMELINE_SUBMITTED] != 0;
}

void investigateLocation(GameState *game)
{
    int locationID;
    Location *location;
    int choice;

    if (game == NULL) {
        return;
    }

    clearScreen();
    locationID = game->playerLocation;
    location = getLocation(locationID);
    if (location == NULL) {
        printf("Unknown location.\n");
        waitForEnter();
        return;
    }

    printf("\n========== INVESTIGATE LOCATION ==========\n");
    displayLocation(locationID);

    /* Do not spend another investigation on a room already searched. */
    if (game->flags[FLAG_ROOM_VISITED_BASE + locationID]) {
        printf("You have already investigated this room.\n");
        waitForEnter();
        return;
    }

    if (game->flags[FLAG_ROOM_COUNT] >= MAX_INVESTIGATED_ROOMS) {
        printf("INVESTIGATION LIMIT REACHED: You have searched all %d rooms allowed.\n",
               MAX_INVESTIGATED_ROOMS);
        waitForEnter();
        return;
    }

    if (locationID > LOCKER_CLUE_COUNT) {
        printf("No essential case evidence is currently associated with this room.\n");
        printf("Searching it will still use one of your six investigation opportunities.\n");
    }

    printf("1. Search this location\n");
    printf("0. Leave without investigating\n");
    choice = readChoice("Choice: ");

    if (choice == 0) {
        return;
    }
    if (choice != 1) {
        printf("Invalid choice.\n");
        waitForEnter();
        return;
    }

    /* Mark this room as searched and increase the investigation count. */
    game->flags[FLAG_ROOM_VISITED_BASE + locationID] = 1;
    game->flags[FLAG_ROOM_COUNT]++;
    printf("Room investigation used: %d/%d\n",
           game->flags[FLAG_ROOM_COUNT], MAX_INVESTIGATED_ROOMS);

    /* Only the six designated rooms contain essential evidence and code digits. */
    {
        static const int evidenceByRoom[LOCKER_CLUE_COUNT + 1] = {
            0,
            EVID_CALL_RECORDING, /* Room 1: Flat 4B */
            EVID_CCTV,           /* Room 2: Corridor */
            EVID_ACCESS_LOG,     /* Room 3: Security Room */
            EVID_SLIP_MARKS,     /* Room 4: Stairwell */
            EVID_GATE_LOG,       /* Room 5: Entrance Gate */
            EVID_COMPLAINT       /* Room 6: Manager's Office */
        };

        if (locationID <= LOCKER_CLUE_COUNT) {
            addEvidence(game, evidenceByRoom[locationID]);
            revealLockerDigit(game, locationID);
        } else {
            printf("You find no case-relevant evidence here.\n");
        }
    }

    /* The concealed locker is behind the painting in Room 6. */
    discoverLocker(game, locationID);

    if (game->flags[FLAG_ROOM_COUNT] >= MAX_INVESTIGATED_ROOMS) {
        printf("\nINVESTIGATION LIMIT REACHED. Rooms you did not search are now locked for Phase 1.\n");
    }
    waitForEnter();
}

void detectiveMode(GameState *game)
{
    int choice;

    if (game == NULL) {
        return;
    }

    initializeMap();

    while (1) {
        clearScreen();
        Location *currentLocation = getLocation(game->playerLocation);

        printf("\n========== DETECTIVE MODE ==========\n");
        if (currentLocation != NULL) {
            printf("Current location: %s\n", currentLocation->name);
        }
        printf("Rooms investigated: %d/%d\n",
               game->flags[FLAG_ROOM_COUNT], MAX_INVESTIGATED_ROOMS);
        printf("1. View building map\n");
        printf("2. Move to another location\n");
        printf("3. Investigate current location\n");
        printf("4. View collected evidence\n");
        printf("5. Check investigation progress\n");
        printf("6. View locker clues / enter combination\n");
        printf("0. Return to game manager\n");

        choice = readChoice("Choice: ");

        clearScreen();
        switch (choice) {
            case 1:
                displayMap();
                waitForEnter();
                break;

            case 2: {
                int destinationID;
                displayMap();
                destinationID = readChoice("Destination ID: ");

                if (destinationID >= 1 && destinationID <= LOCATION_COUNT) {
                    if (moveToLocation(game->playerLocation, destinationID)) {
                        game->playerLocation = destinationID;
                    }
                } else {
                    printf("Invalid location ID.\n");
                }
                waitForEnter();
                break;
            }

            case 3:
                investigateLocation(game);
                break;

            case 4:
                showEvidenceInventory(game);
                waitForEnter();
                break;

            case 5: {
                int evidenceCount = 0;
                const EvidenceNode *item = game->evidence;

                while (item != NULL) {
                    evidenceCount++;
                    item = item->next;
                }

                printf("Rooms investigated: %d/%d\n",
                       game->flags[FLAG_ROOM_COUNT], MAX_INVESTIGATED_ROOMS);
                printf("Evidence collected: %d item(s).\n", evidenceCount);
                if (game->flags[FLAG_ROOM_COUNT] >= MAX_INVESTIGATED_ROOMS) {
                    printf("Uninvestigated rooms are locked. Choose suspects and evidence in Timeline.\n");
                }
                waitForEnter();
                break;
            }

            case 6:
                if (!game->lockerFound) {
                    printf("The locker has not been discovered. Search the painting in the Manager's Office (Room 6).\n");
                } else if (game->playerLocation != LOCKER_ROOM_ID) {
                    displayLockerClues(game);
                    printf("Return to the Manager's Office (Room 6) to enter the combination.\n");
                } else {
                    (void)useLocker(game);
                }
                waitForEnter();
                break;

            case 0:
                return;

            default:
                printf("Invalid choice. Try again.\n");
                waitForEnter();
                break;
        }
    }
}
