#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <omp.h>

#include "matrix/matrix.h"
#include "output/output.h"
#include "solver/solver.h"

#define ROWS 12
#define COLS 12

void (* solve)(int rows, int cols, int iterations, struct timespec ts_sleep, int ** matrix) = solveFirst;

int main(int argc, char* argv[]) {
    if(5 != argc) {
        return EXIT_FAILURE;
    }

    struct timespec ts_sleep;
    ts_sleep.tv_sec = 0;
    ts_sleep.tv_nsec = 1000L;

    int nbThreads = atoi(argv[1]);
    int problem = atoi(argv[2]);
    int initialValue = atoi(argv[3]);
    int iterations = atoi(argv[4]);

    void * solvers[2];
    solvers[0] = solveFirst;
    solvers[1] = solveSecond;

    solve = solvers[problem - 1];

    int ** matrix = allocateMatrix(ROWS, COLS);

    // Sequential
    struct timeval timestamp_s_seq;
    struct timeval timestamp_e_seq;

    omp_set_num_threads(1);
    fillMatrix(ROWS, COLS, initialValue, matrix);

    gettimeofday(&timestamp_s_seq, NULL);
    solve(ROWS, COLS, iterations, ts_sleep, matrix);
    gettimeofday(&timestamp_e_seq, NULL);
    
    printMatrix(ROWS, COLS, matrix);

    // Parallel
    struct timeval timestamp_s_par;
    struct timeval timestamp_e_par;

    omp_set_num_threads(nbThreads);
    fillMatrix(ROWS, COLS, initialValue, matrix);

    gettimeofday(&timestamp_s_par, NULL);
    solve(ROWS, COLS, iterations, ts_sleep, matrix);
    gettimeofday(&timestamp_e_par, NULL);
    
    printMatrix(ROWS, COLS, matrix);

    // Statistics
    printStatistics(nbThreads, timestamp_s_seq, timestamp_e_seq, timestamp_s_par, timestamp_e_par);
    deallocateMatrix(ROWS, matrix);

    return EXIT_SUCCESS;
}
