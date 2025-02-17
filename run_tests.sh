#!/bin/bash
rm -f sudoku
echo "Compiling sudoku program..."
gcc -Wall -Wextra -pthread -lm sudoku.c -o sudoku

echo -e "\nRunning tests..."

declare -a tests=(
    "test1_4x4_valid.txt"
    "test2_9x9_valid.txt"
    "test3_4x4_invalid_row.txt"
    "test4_9x9_invalid_column.txt"
    "test5_4x4_invalid_subgrid.txt"
    "test6_4x4_incomplete.txt"
    "test7_9x9_incomplete.txt"
    "test8_16x16_valid.txt"
)

for test in "${tests[@]}"; do
    echo -e "\nTesting $test"
    echo "------------------------"
    ./sudoku "$test"
done

echo -e "\nTests completed."