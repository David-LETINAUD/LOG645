#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>

#include "matrix/matrix.h"
#include "output/output.h"
#include "solver/solver.h"

#define DIM 64

int (* solve)(int iterations, int rank, int value) = solveFirst;

int main(int argc, char* argv[]) {
    struct timeval timestamp_s;
    struct timeval timestamp_e;

    int problem = atoi(argv[1]);
    int initialValue = atoi(argv[2]);
    int iterations = atoi(argv[3]);

    void * solvers[2];
    solvers[0] = solveFirst;
    solvers[1] = solveSecond;

    solve = solvers[problem - 1];

	int number;
	int world_rank;
    int *rbuf = NULL;

    MPI_Init(NULL, NULL);
	gettimeofday(&timestamp_s, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	if (world_rank == 0) {
    	rbuf = malloc(sizeof(int) * DIM);
    }
	
	number = solve(iterations, world_rank, initialValue);
	
    MPI_Gather(&number, 1, MPI_INT, rbuf, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
		if (problem == 1) {
			printMatrix1(DIM, rbuf);
		} else {
			printMatrix2(DIM, rbuf);
		}
		
		gettimeofday(&timestamp_e, NULL);
		printRuntime(timestamp_s, timestamp_e);
    }
    
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}
