#include <stdio.h>
#include <stdlib.h>
#include "futo.h"

int main() {
    char filename[20];
    int *puzzle;       /* array for puzzle numbers */
    int *constraints;  /* array for puzzle constraints */
    int size;          /* size of puzzle */
    printf("Puzzle file: ");
    fflush(stdout);
    scanf("%20s", filename);

    if (!readPuzzle(filename, &puzzle, &constraints, &size)) {
        printf("Unable to open puzzle file: %s\n", filename);
        return 1;
    }
    printf("PUZZLE:\n");
    printPuzzle(puzzle, constraints, size);

    printf("\nSOLUTION:\n");
    if (solve(puzzle, constraints, size)) {
        printPuzzle(puzzle, constraints, size);
    }
    else {
        printf("No solution exists.\n");
    }
    return EXIT_SUCCESS;
}

