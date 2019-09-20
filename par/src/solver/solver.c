#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

#include "solver.h"

void solveFirst(int iterations, int rank, int * matrix) {
	int sum = rank % 8 + rank / 8;

    for(int k = 1; k <= iterations; ++k) {
    	// usleep(1000);
        matrix[rank] = matrix[rank] + sum * k;
    }
}

void solveSecond(int iterations, int rank, int * matrix) {
	if (rank < 8) {
		for (int k = 1; k <= iterations; ++k) {
			// usleep(1000);
			matrix[rank] = matrix[rank] + rank * k;
			MPI_Send(&matrix[rank], 1, MPI_INT, rank + 8, k, MPI_COMM_WORLD);
		}
	} else {
		int i = rank % 8;
		
		int number = 0;
		
		for (int k = 1; k <= iterations; ++k) {
			MPI_Recv(&number, 1, MPI_INT, rank - 8, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			
			// usleep(1000);
			int p = i + rank;
			matrix[p] = matrix[p] + matrix[p - 8];

			if (rank < 56) {
				MPI_Send(&matrix[p], 1, MPI_INT, rank + 8, k, MPI_COMM_WORLD);
			}
		}
	}
}
