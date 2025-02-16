// Sudoku puzzle verifier and solver

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

bool checkRows(int psize, int **grid) {
    for (int row = 1; row <= psize; row++) {
        bool found[psize + 1];  // To track numbers 1 to psize
        for (int i = 1; i <= psize; i++) {
            found[i] = false;
        }

        for (int col = 1; col <= psize; col++) {
            int num = grid[row][col];
            if (num < 1 || num > psize || found[num]) {
                return false; // Duplicate or invalid number found
            }
            found[num] = true;
        }
    }
    return true; // All rows are valid
}

bool checkColumns(int psize, int **grid) {
    for (int col = 1; col <= psize; col++) {
        bool found[psize + 1];  // To track numbers 1 to psize
        for (int i = 1; i <= psize; i++) {
            found[i] = false;
        }

        for (int row = 1; row <= psize; row++) {
            int num = grid[row][col];
            if (num < 1 || num > psize || found[num]) {
                return false; // Duplicate or invalid number found
            }
            found[num] = true;
        }
    }
    return true; // All columns are valid
}

// Function to check if all subgrids in the Sudoku puzzle are valid
bool checkSubgrids(int psize, int **grid) {
    int subgrid_size = sqrt(psize);  // Each subgrid is subgrid_size x subgrid_size

    // Iterate through each subgrid
    for (int startRow = 1; startRow <= psize; startRow += subgrid_size) {
        for (int startCol = 1; startCol <= psize; startCol += subgrid_size) {
            bool found[psize + 1];  // Track numbers 1 to psize
            for (int i = 1; i <= psize; i++) {
                found[i] = false;
            }

            // Check all numbers inside the subgrid
            for (int row = startRow; row < startRow + subgrid_size; row++) {
                for (int col = startCol; col < startCol + subgrid_size; col++) {
                    int num = grid[row][col];
                    if (num < 1 || num > psize || found[num]) {
                        return false;  // Duplicate or invalid number found
                    }
                    found[num] = true;
                }
            }
        }
    }
    return true;  // All subgrids are valid
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  // YOUR CODE GOES HERE and in HELPER FUNCTIONS
  *valid = true;
  *complete = true;

    // Check if the puzzle is complete (no zeros)
    for (int row = 1; row <= psize; row++) {
        for (int col = 1; col <= psize; col++) {
            if (grid[row][col] == 0) {
                *complete = false;
                return; // No need to check validity if incomplete
            }
        }
    }

    // Check if rows are valid
    if (!checkRows(psize, grid)) {
        *valid = false;
        return;
    }

    // Check if columns are valid
    if (!checkColumns(psize, grid)) {
        *valid = false;
        return;
    }

    // Check if subgrids are valid
    if (!checkSubgrids(psize, grid)) {
        *valid = false;
        return;
    }
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
