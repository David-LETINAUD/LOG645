#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

#include "solver.h"

void solveFirst(int iterations, int rank, int * matrix) {
	int sum = rank % 8 + rank / 8;

    for(int k = 1; k <= iterations; ++k) {
    	// usleep(1000);
        matrix[rank] += sum * k;
    }
}

void solveSecond(int iterations, int rank, int * matrix) {
	if (rank < 8) {
		for (int k = 1; k <= iterations; ++k) {
			// usleep(1000);
			matrix[rank] += rank * k;
			MPI_Send(&matrix[rank], 1, MPI_INT, rank + 8, 0, MPI_COMM_WORLD);
		}
	} else {
		int number = 0;
		
		for (int k = 1; k <= iterations; ++k) {
			// usleep(1000);
			MPI_Recv(&number, 1, MPI_INT, rank - 8, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			matrix[rank] += number * k;
			
			if (rank < 56) {
				MPI_Send(&matrix[rank], 1, MPI_INT, rank + 8, 0, MPI_COMM_WORLD);
			}
		}
	}
}
