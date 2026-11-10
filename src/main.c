#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "screen.h"

static int readMenuChoice(const char *prompt)
{
    char line[64];
    char *end;
    long value;

    printf("%s", prompt);
    if (fgets(line, sizeof line, stdin) == NULL)
        return -1;

    value = strtol(line, &end, 10);
    if (end == line || value < 0 || value > 3)
        return -1;

    while (*end == ' ' || *end == '\t')
        end++;

    if (*end != '\n' && *end != '\0')
        return -1;

    return (int)value;
}

int main(void)
{
    int choice;

    for (;;) {
        clearScreen();
        printf("\n========== MURDER MYSTERY ==========\n");
        printf("1. New Game\n");
        printf("2. Load Game\n");
        printf("3. Exit\n");

        choice = readMenuChoice("Enter your choice: ");

        clearScreen();
        switch (choice) {
            case 1:
                rungame();
                break;
            case 2:
                runLoadedGame();
                break;
            case 3:
                printf("Goodbye.\n");
                return 0;
            default:
                printf("Please enter 1, 2, or 3.\n");
                waitForEnter();
                break;
        }
    }
}