#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "matrix/matrix.h"
#include "output/output.h"
#include "solver/solver.h"

#define ROWS 8
#define COLS 8

void (* solve)(int rows, int cols, int iterations, struct timespec ts_sleep, int ** matrix) = solveFirst;

int main(int argc, char* argv[]) {
    if(4 != argc) {
        return EXIT_FAILURE;
    }

    struct timeval timestamp_s;
    struct timeval timestamp_e;

    struct timespec ts_sleep;
    ts_sleep.tv_sec = 0;
    ts_sleep.tv_nsec = 1000000L;

    int problem = atoi(argv[1]);
    int initialValue = atoi(argv[2]);
    int iterations = atoi(argv[3]);

    void * solvers[2];
    solvers[0] = solveFirst;
    solvers[1] = solveSecond;

    solve = solvers[problem - 1];

    int ** matrix = allocateMatrix(ROWS, COLS);
    fillMatrix(ROWS, COLS, initialValue, matrix);

    gettimeofday(&timestamp_s, NULL);
    solve(ROWS, COLS, iterations, ts_sleep, matrix);
    gettimeofday(&timestamp_e, NULL);
    
    printMatrix(ROWS, COLS, matrix);
    printRuntime(timestamp_s, timestamp_e);
    deallocateMatrix(ROWS, matrix);

    return EXIT_SUCCESS;
}
