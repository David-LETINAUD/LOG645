#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "solver.h"

void solveFirst(int rows, int cols, int iterations, struct timespec ts_sleep, int ** matrix) {
    for(int k = 1; k <= iterations; k++) {
        for(int j = 0; j < cols; j++) {
            for(int i = 0; i < rows; i++) {
                nanosleep(&ts_sleep, NULL);
                matrix[i][j] = matrix[i][j] + (i + j) * k;
            }
        }
    }
}

void solveSecond(int rows, int cols, int iterations, struct timespec ts_sleep, int ** matrix) {
    for(int k = 1; k <= iterations; k++) {
        for(int i = 0; i < rows; i++) {
            nanosleep(&ts_sleep, NULL);
            matrix[i][0] = matrix[i][0] + (i * k);
        }

        for(int j = 1; j < cols; j++) {
            for(int i = 0; i < rows; i++) {
                nanosleep(&ts_sleep, NULL);
                matrix[i][j] = matrix[i][j] + matrix[i][j - 1] * k;
            }
        }
    }
}
