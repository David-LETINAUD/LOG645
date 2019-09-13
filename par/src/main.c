#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>

#include "matrix/matrix.h"
#include "output/output.h"
#include "solver/solver.h"

#define DIM 64

void (* solve)(int iterations, int rank, int * matrix) = solveFirst;

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

    int * matrix = allocateMatrix(DIM);
    fillMatrix(DIM, initialValue, matrix);

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

    gettimeofday(&timestamp_s, NULL);

    solve(iterations, world_rank, matrix);

    if (problem == 1) {
    	MPI_Gather(&matrix[world_rank], 1, MPI_INT, rbuf, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
    	MPI_Gather(&matrix[world_rank], 8, MPI_INT, rbuf, 8, MPI_INT, 0, MPI_COMM_WORLD);
    }

    gettimeofday(&timestamp_e, NULL);

    // Finalize the MPI environment.
    MPI_Finalize();

    if (world_rank == 0) {
		printMatrix(DIM, rbuf);
		printRuntime(timestamp_s, timestamp_e);
		deallocateMatrix(DIM, matrix);
    }

    return EXIT_SUCCESS;
}
