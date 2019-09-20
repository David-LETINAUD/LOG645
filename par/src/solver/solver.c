#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

#include "solver.h"

int solveFirst(int iterations, int rank, int value) {
	int sum = rank % 8 + rank / 8;

    for(int k = 1; k <= iterations; ++k) {
    	usleep(1000);
        value += sum * k;
    }

    return value;
}

int solveSecond(int iterations, int rank, int value) {
	if (rank < 8) {
		for (int k = 1; k <= iterations; ++k) {
			usleep(1000);
			value += rank * k;
			
			MPI_Send(&value, 1, MPI_INT, rank + 8, 0, MPI_COMM_WORLD);
		}
	} else {
		int number = 0;
		
		for (int k = 1; k <= iterations; ++k) {
			MPI_Recv(&number, 1, MPI_INT, rank - 8, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			
			usleep(1000);
			value += number * k;
			
			if (rank < 56) {
				MPI_Send(&value, 1, MPI_INT, rank + 8, 0, MPI_COMM_WORLD);
			}
		}
	}
	
	return value;
}
