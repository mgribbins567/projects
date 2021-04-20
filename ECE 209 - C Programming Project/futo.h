//Remember your include statements

#ifndef FUTO_H


/*
 * This function attempts to solve a puzzle, represented by the
 * puzzle array and constraints array, with the given size.
 *
 * It solves the puzzle by filling in the puzzle array with numbers.
 * Refer to the provided pdf to understand how this function is to be implemented.
 *
 * returns 1 if the solution is successful, and 0 if there is no solution.
 */
int solve(int puzzle[], const int constraints[], int size);

/*
 * This function is to (a) open the file,
 * (b) read the puzzle description and create the data structures,
 * (c) use the pointer arguments to “return” those data structure to the caller,
 * and (d) close the file.
 * Parameters:
 * size  - size of the puzzle.
 * puzzle - array to hold interger to solve the puzzle
 * constraints - array to hold the constraints of the puzzle
 *
 * The return value is 1 if the file was read successfully, and 0 otherwise.
 *
 * Array needs to be dynamically allocated and stored using the int** provided by the caller.
 * The array must contain zeroes for boxes with no constraints.
 * The encoded constraints for those boxes for which relational constraints are specified in the pdf file.
 */
int readPuzzle(const char *filename, int **puzzle, int **constraints, int *size);

/*
 * This function determines whether it is
 * allowed to place a particular number (num) into a
 * particular box of the puzzle grid,
 * given the row, column and constraints.
 *
 * returns 1 if the number is legal, and 0 otherwise.
 */
int isLegal(int row, int col, int num, const int puzzle[], const int constraints[], int size);

/*
 * This function prints the puzzle to the console. (prinf)
 *
 * Refer to the provides pdf to formatting specifications
 */
void printPuzzle(const int puzzle[], const int constraints[], int size);


#define FUTO_H

#endif //FUTO_H
