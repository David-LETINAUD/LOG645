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
    if(4 != argc) {
        return EXIT_FAILURE;
    }

    struct timeval timestamp_s;
    struct timeval timestamp_e;

    int problem = atoi(argv[1]);
    int initialValue = atoi(argv[2]);
    int iterations = atoi(argv[3]);

    void * solvers[2];
    solvers[0] = solveFirst;
    solvers[1] = solveSecond;

    solve = solvers[problem - 1];

	gettimeofday(&timestamp_s, NULL);

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int *rbuf = NULL;

    if (world_rank == 0) {
    	rbuf = malloc(sizeof(int) * DIM);
    }

	int number = solve(iterations, world_rank, initialValue);

    MPI_Gather(&number, 1, MPI_INT, rbuf, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Finalize the MPI environment.
    MPI_Finalize();

    if (world_rank == 0) {
		printMatrix(DIM, rbuf);
		printRuntime(timestamp_s, timestamp_e);
    }
    
    gettimeofday(&timestamp_e, NULL);

    return EXIT_SUCCESS;
}
