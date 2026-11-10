#ifndef SCREEN_H
#define SCREEN_H

#include <stdio.h>
#include <stdlib.h>

/* Clears terminal output only. It does not alter game state or save data. */
static void clearScreen(void)
{
#ifdef _WIN32
    (void)system("cls");
#else
    (void)system("clear");
#endif
}

/* Keep operation results visible until the player is ready to continue. */
static void waitForEnter(void)
{
    char line[8];
    printf("\nPress Enter to continue...");
    (void)fgets(line, sizeof line, stdin);
}

#endif
