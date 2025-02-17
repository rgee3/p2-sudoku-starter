// Sudoku puzzle verifier and solver
// Compilation and Running Instructions:
//
//1. To compile:
//   gcc -Wall -Wextra -pthread -lm -std=c99 sudoku.c -o sudoku
//
//2. To run with a single puzzle file:
//   ./sudoku puzzle-file.txt
//
//   Example:
//   ./sudoku test1_4x4_valid.txt
//
//3. To run all tests using the test script:
//   First make the script executable:
//   chmod +x run_tests.sh
//
//   Then run it:
//   ./run_tests.sh
//
// See README for more info

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int psize;  // puzzle size (e.g. 9x9)
    int **grid; // 2D array of Sudoku grid
    bool result; // validation result
} ThreadData;

// This function validates all rows in a Sudoku puzzle by checking if each row contains
// unique numbers from 1 to `psize` (puzzle size) with no duplicates
void *checkRows(void *threadInfo) {
    ThreadData *data = (ThreadData *)threadInfo;
    data->result = true;

    // Track which numbers have been seen in a row
    bool *found = (bool *)malloc((data->psize + 1) * sizeof(bool));
    if (found == NULL) {
        data->result = false;
        pthread_exit(NULL);
    }

    // Check each row
    for (int row = 1; row <= data->psize; row++) {
        for (int i = 1; i <= data->psize; i++) found[i] = false;    // reset before checking new row

        for (int col = 1; col <= data->psize; col++) {
            int num = data->grid[row][col];
            if (num < 1 || num > data->psize || found[num]) {   // invalid if num is out of range or already in row
                data->result = false;
                free(found);
                pthread_exit(NULL);
            }
            found[num] = true; // mark number as seen
        }
    }

    free(found);
    pthread_exit(NULL);
}

// This function validates all columns in a Sudoku puzzle by checking if each row contains
// unique numbers from 1 to `psize` (puzzle size) with no duplicates
void *checkColumns(void *threadInfo) {
    ThreadData *data = (ThreadData *)threadInfo;
    data->result = true;

    // track which numbers have been seen in a column
    bool *found = (bool *)malloc((data->psize + 1) * sizeof(bool));
    if (found == NULL) {
        data->result = false;
        pthread_exit(NULL);
    }

    for (int col = 1; col <= data->psize; col++) {
        for (int i = 1; i <= data->psize; i++) found[i] = false;    // reset array before checking new column

        for (int row = 1; row <= data->psize; row++) {
            int num = data->grid[row][col];
            if (num < 1 || num > data->psize || found[num]) {   // invalid if out of range or already found in column
                data->result = false;
                free(found);
                pthread_exit(NULL);
            }
            found[num] = true; // mark as seen in column
        }
    }

    free(found);
    pthread_exit(NULL);
}

// This function validates all sub-grids in a Sudoku puzzle; it ensures that
// each `sqrt(psize) x sqrt(psize)` sub-grid contains unique numbers from 1 to `psize` (puzzle size),
// with no duplicates.
// (Example sub-grids: if large grid is 9x9, sub-grid is 3x3; if larger grid is 16, sub-grid is 4x4)
void *checkSubgrids(void *threadInfo) {
    ThreadData *data = (ThreadData *)threadInfo;
    data->result = true;

    // Calculate sub-grid size
    int subgrid_size = sqrt(data->psize);

    // Track which numbers have been seen in a subgrid
    bool *found = (bool *)malloc((data->psize + 1) * sizeof(bool));
    if (found == NULL) {
        data->result = false;
        pthread_exit(NULL);
    }

    // Loop through sub-grid
    for (int startRow = 1; startRow <= data->psize; startRow += subgrid_size) {
        for (int startCol = 1; startCol <= data->psize; startCol += subgrid_size) {
            for (int i = 1; i <= data->psize; i++) found[i] = false; // reset before checking new sub-grid

            for (int row = startRow; row < startRow + subgrid_size; row++) {
                for (int col = startCol; col < startCol + subgrid_size; col++) {
                    int num = data->grid[row][col];
                    if (num < 1 || num > data->psize || found[num]) { // invalid if num is out of valid range or already found in subgrid
                        data->result = false;
                        free(found);
                        pthread_exit(NULL);
                    }
                    found[num] = true; // mark num as seen
                }
            }
        }
    }

    free(found);
    pthread_exit(NULL);
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  *valid = true;
  *complete = true;

// Check if puzzle is 'complete' (i.e., no zeroes)
    for (int row = 1; row <= psize; row++) {
        for (int col = 1; col <= psize; col++) {
            if (grid[row][col] == 0) {
                *complete = false;
                return; // if incomplete, we don't check validity
            }
        }
    }

    // Thread data structs with row, column, and sub-grid validation
    ThreadData rowData = {psize, grid, false};
    ThreadData colData = {psize, grid, false};
    ThreadData subData = {psize, grid, false};

    // Create threads
    pthread_t rowThread, colThread, subThread;

    if (pthread_create(&rowThread, NULL, checkRows, &rowData) != 0) {
        perror("Failed to create row thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&colThread, NULL, checkColumns, &colData) != 0) {
        perror("Failed to create column thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&subThread, NULL, checkSubgrids, &subData) != 0) {
        perror("Failed to create subgrid thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(rowThread, NULL);
    pthread_join(colThread, NULL);
    pthread_join(subThread, NULL);

    // Check if validation tests failed
    if (!rowData.result || !colData.result || !subData.result) {
        *valid = false;
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
