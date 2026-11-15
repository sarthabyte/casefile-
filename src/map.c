#include <stdio.h>
#include <string.h>
#include "map.h"

/* The locations array is shared only inside this file. */
static Location locations[LOCATION_COUNT];

/* Fill one location without using advanced C initializer syntax. */
static void setLocation(int arrayIndex, int id, const char *name,
                        const char *description, int first, int second,
                        int third, int fourth, int connectionCount)
{
    locations[arrayIndex].id = id;
    snprintf(locations[arrayIndex].name, sizeof(locations[arrayIndex].name), "%s", name);
    snprintf(locations[arrayIndex].description,
             sizeof(locations[arrayIndex].description), "%s", description);

    locations[arrayIndex].connections[0] = first;
    locations[arrayIndex].connections[1] = second;
    locations[arrayIndex].connections[2] = third;
    locations[arrayIndex].connections[3] = fourth;
    locations[arrayIndex].connectionCount = connectionCount;
}

void initializeMap(void)
{
    /* Each call sets one location and the IDs of its connected locations. */
    setLocation(0, 1, "Flat 4B - Crime Scene", "Meena's apartment and the first crime scene.", 2, 7, 0, 0, 2);
    setLocation(1, 2, "Fourth-floor Corridor", "A long corridor with several doors and a blind camera spot.", 1, 3, 4, 8, 4);
    setLocation(2, 3, "Security Room", "Security equipment and access-card records.", 2, 5, 6, 9, 4);
    setLocation(3, 4, "Stairwell", "The stairwell between the fourth and fifth floors.", 2, 5, 8, 0, 3);
    setLocation(4, 5, "Entrance Gate", "The main entrance and visitor records.", 3, 4, 10, 0, 3);
    setLocation(5, 6, "Manager's Office", "Building management records, complaints, and a framed landscape painting on the wall.", 3, 9, 11, 0, 3);
    setLocation(6, 7, "Kitchen", "A small kitchen with a sink and a back service door.", 1, 8, 12, 0, 3);
    setLocation(7, 8, "Laundry Room", "A shared laundry room beside the service stairs.", 2, 4, 7, 9, 4);
    setLocation(8, 9, "Maintenance Closet", "Tools, spare locks, and maintenance logs are stored here.", 3, 6, 8, 10, 4);
    setLocation(9, 10, "Parking Garage", "A dim underground garage with vehicle spaces.", 5, 9, 11, 0, 3);
    setLocation(10, 11, "Rooftop Access", "A locked access landing near the roof door.", 6, 10, 12, 0, 3);
    setLocation(11, 12, "Guest Room", "An unused guest room with scattered personal belongings.", 7, 11, 0, 0, 2);
}

/* Return a location by ID. IDs start at 1, but array indexes start at 0. */
Location *getLocation(int locationID)
{
    if (locationID < 1 || locationID > LOCATION_COUNT) {
        return NULL;
    }

    return &locations[locationID - 1];
}

void displayMap(void)
{
    int i;

    printf("\n========== BUILDING MAP (12 LOCATIONS) ==========\n");
    for (i = 0; i < LOCATION_COUNT; i++) {
        printf("%2d. %s\n", locations[i].id, locations[i].name);
    }
    printf("=================================================\n");
}

void displayConnectionTable(void)
{
    int i;

    printf("\n========== MOVE TO ANOTHER LOCATION ==========\n");
    printf("  Room ID   Room Name                        Connected To        \n");
    for (i = 1; i <= LOCATION_COUNT; i++) {
        Location *location = getLocation(i);
        int j;

        printf("  %7d   %-30.30s  ", location->id, location->name);
        for (j = 0; j < location->connectionCount; j++)
            printf(" %d", location->connections[j]);
        for (; j < 4; j++)
            printf("   ");
        printf("\n");
    }
    printf("Enter a destination ID (0 cancels): ");
}

void displayLocation(int locationID)
{
    Location *location = getLocation(locationID);
    int i;

    if (location == NULL) {
        printf("Invalid location.\n");
        return;
    }

    printf("\nLocation: %s\n", location->name);
    printf("%s\n", location->description);
    printf("Connected locations: ");

    for (i = 0; i < location->connectionCount; i++) {
        printf("%d ", location->connections[i]);
    }
    printf("\n");
}

/* A move is allowed only when the destination is in the current location's list. */
int moveToLocation(int currentID, int targetID)
{
    Location *current = getLocation(currentID);
    Location *target = getLocation(targetID);
    int i;

    if (current == NULL || target == NULL) {
        printf("Invalid location ID.\n");
        return 0;
    }

    for (i = 0; i < current->connectionCount; i++) {
        if (current->connections[i] == targetID) {
            printf("You moved to %s.\n", target->name);
            return 1;
        }
    }

    printf("You cannot move directly to that location.\n");
    return 0;
}
