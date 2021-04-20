/* Program 2 -- Futoshiki puzzle */
/* Matthew Gribbins
 * Program 2
 * Takes an input Futoshiki puzzle and solves it
*/

#include "futo.h"
#include <stdio.h>
#include <stdlib.h>
/* Reads puzzle from a file that is found through input from user
*/
int readPuzzle(const char *filename, int **puzzle, int **constraints, int *size) {
    FILE *inf;
    int s, *p, *c, i,j;
    char cin=0;
    inf = fopen(filename,"r");
    if(inf==NULL)
        return 0;

/* Scans for the size value of the puzzle */

    fscanf(inf, "%d", size);
    s = *size;

    p = (int*) malloc(s*s*sizeof(int));
    c = (int*) malloc(s*s*sizeof(int));
    for(i=0; i<s*s;i++){
        c[i]=0;
    }
    i=0;
    while(!feof(inf) && (i<s)) {
        j=0;
        while (cin != '|')
            fscanf(inf, "%c", &cin);
        cin = 0;
        while (cin != '|') {
            fscanf(inf, "%c", &cin);
            if (cin == '-') {
                p[i * s + j] = 0;
                j++;
            }else if ((cin >= '1') && (cin <= '9')) {
                p[i * s + j] = cin - '0';
                j++;
            } else if (cin == '>') {
                c[i * s + j - 1] += 2;
            } else if (cin == '<') {
                c[i * s + j - 1] += 1;
            } else if (cin == ' ') {}
        }
        cin = 0;
        while ((cin != '|') && (i < s - 1))
            fscanf(inf, "%c", &cin);
        cin = 0;
        j = 0;
        while ((cin != '|') && (i < s - 1)) {
            fscanf(inf, "%c", &cin);
            if (cin == '^') {
                c[i * s + j] += 4;
                if (j < s - 1)
                    fscanf(inf, "%c", &cin);
                j++;
            } else if (cin == 'v') {
                c[i * s + j] += 8;
                if (j < s - 1)
                    fscanf(inf, "%c", &cin);
                j++;
            } else if (cin == ' ') {
                if (j < s - 1)
                    fscanf(inf, "%c", &cin);
                j++;
            }
        }
        cin=0;
        i++;
    }

    *constraints = c;
    *puzzle = p;
    fclose(inf);
    return 1;
}

/* Function that prints the puzzle after it is read from file */
void printPuzzle(const int puzzle[], const int constraints[], int size) {
    int i, j;
    for (j = 0; j < size; j++) {
        printf("|");
        for (i = j*size; i < j*size+size; i++) {
            if (puzzle[i] == 0) {
                printf("-");
            }
            else {
                printf("%d", puzzle[i]);
            }
            if (constraints[i] - 8 == 1 || constraints[i] == 1) {
                printf("<");
            }
            else if (constraints[i] - 8 == 2 || constraints[i] == 2) {
                printf(">");
            }
            else if (i != j * size + size - 1){
                printf(" ");
            }
        }
        printf("|\n");
        if (j != size-1) {
            printf("|");
            for (i = j * size; i < j * size + size; i++) {
                if (constraints[i] == 0 || constraints[i] == 1 || constraints[i] == 2) {
                    printf(" ");
                } else if (constraints[i] - 1 == 4 || constraints[i] - 2 == 4 || constraints[i] - 3 == 4 || constraints[i] == 4) {
                    printf("^");
                } else if (constraints[i] - 1 == 8 || constraints[i] - 2 == 8|| constraints[i] - 3 == 8 || constraints[i] == 8) {
                    printf("v");
                }
                if (i != j * size + size - 1) {
                    printf(" ");
                }
            }
            printf("|\n");
        }
    }
}

/* Solves the puzzle recursively by incremently stepping through the puzzle and trying each value from 1 to the size */
int solve(int puzzle[], const int constraints[], int size) {
    int i, j, row, col;
    for (i = 0; i < size*size; i++) {
        if (puzzle[i] == 0) {
            for (j = 1; j <= size; j++) {
                row = i / size;
                col = i % size;
                if (isLegal(row, col, j, puzzle, constraints, size) == 1) {
                    puzzle[i] = j;
                    if (solve(puzzle, constraints, size) == 1) {
                        return 1;
                    }
                    else {
                        puzzle[i] = 0;
                    }
                }
            }
        }
        else if (i == (size*size - 1)) {
            return 1;
        }
    }
    return 0;
}

/* Checks if the entered num value is valid by checking the constraints around it alongside the rows and column values */
int isLegal(int row, int col, int num, const int puzzle[], const int constraints[], int size) {
    int i;
    int j = 0;
    if (num == 0) {
        return 0;
    }
    for (i = size*row; i < size*row + size; i++) {
        if (num == puzzle[i]) {
            j++;
        }
    }
    if (j > 0) {
        return 0;
    }
    j = 0;
    for (i = col; i < size*size; i += size) {
        if (num == puzzle[i]) {
            j++;
        }
    }
    if (j > 0) {
        return 0;
    }
    i = row*size + col;
    if (constraints[i] == 1 || constraints[i] == 5 || constraints[i] == 9) {
        if (num < puzzle[i + 1] || puzzle[i + 1] == 0) {

        }
        else {
            return 0;
        }
    }
    if (constraints[i] == 2 || constraints[i] == 6 || constraints[i] == 10) {
        if (num > puzzle[i + 1] || puzzle[i + 1] == 0) {

        }
        else {
            return 0;
        }
    }
    if (constraints[i] == 4 || constraints[i] == 5 || constraints[i] == 6) {
        if (num < puzzle[i + size] || puzzle[i + size] == 0) {

        }
        else {
            return 0;
        }
    }
    if (constraints[i] == 8 || constraints[i] == 9 || constraints[i] == 10) {
        if (num > puzzle[i + size] || puzzle[i + size] == 0) {

        }
        else {
            return 0;
        }
    }
    if (col != 0) {
        if (constraints[i - 1] == 1 || constraints[i - 1] == 5 || constraints[i - 1] == 9) {
            if (num > puzzle[i - 1] || puzzle[i - 1] == 0) {

            }
            else {
                return 0;
            }
        }
        if (constraints[i - 1] == 2 || constraints[i - 1] == 6 || constraints[i - 1] == 10) {
            if (num < puzzle[i - 1] || puzzle[i - 1] == 0) {

            }
            else {
                return 0;
            }
        }
    }
    if (row != 0) {
        if (constraints[i - size] == 4 || constraints[i - size] == 5 || constraints[i - size] == 6) {
            if (num > puzzle[i - size] || puzzle[i - size] == 0) {

            }
            else {
                return 0;
            }
        }
        if (constraints[i - size] == 8 || constraints[i - size] == 9 || constraints[i - size] == 10) {
            if (num < puzzle[i - size] || puzzle[i - size] == 0) {

            }
            else {
                return 0;
            }
        }
    }
    return 1;
}
