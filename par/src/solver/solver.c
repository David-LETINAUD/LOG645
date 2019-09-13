#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

#include "solver.h"

void solveFirst(int iterations, int rank, int * matrix) {
	int sum = rank % 8 + rank / 8;

    for(int k = 1; k <= iterations; ++k) {
    	usleep(1000);
        matrix[rank] = matrix[rank] + sum * k;
    }
}

void solveSecond(int iterations, int rank, int * matrix) {
	int i = rank % 8;

    for (int k = 1; k <= iterations; ++k) {
    	usleep(1000);
    	matrix[i] = matrix[i] + i * k;

    	for (int j = 1; j < 8; ++j) {
    		usleep(1000);
    		matrix[i + j] = matrix[i + j] + (matrix[i + j - 1] * k);
    	}
    }
}
