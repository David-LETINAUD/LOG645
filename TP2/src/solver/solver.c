#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "solver.h"

int max(int a, int b);
int min(int a, int b);

#define LAST_COL 11

void solveFirst(const int rows, const int cols, const int iterations, const struct timespec ts_sleep, int **matrix) {
	#pragma omp parallel for collapse(2)
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			nanosleep(&ts_sleep, NULL);
			matrix[i][j] = (i + j)*iterations;
		}
	}
}

void solveSecond(const int rows, const int cols, const int iterations, const struct timespec ts_sleep, int **matrix) {
	#pragma omp parallel for
	for (int i = 0; i < rows; i++) {
		for (int k = 0; k < iterations; k++) {
			
			matrix[i][LAST_COL] += i;
			for (int j = cols - 2; j >= 0; j--) {
				
				matrix[i][j] += matrix[i][j + 1];
			}
		}
	}
}

int max(int a, int b) {
    return a >= b ? a : b;
}

int min(int a, int b) {
    return a <= b ? a : b;
}
