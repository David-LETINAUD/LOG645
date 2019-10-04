#include <stdio.h>
#include <sys/time.h>

void printMatrix(int rows, int cols, int ** matrix) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            printf("%12d ", matrix[i][j]);
        }

        printf("\n");
    }

    printf("\n");
}

void printStatistics(int nbThreads, struct timeval tvs_seq, struct timeval tve_seq, struct timeval tvs_par, struct timeval tve_par) {
    long start = tvs_seq.tv_sec * 1000000 + tvs_seq.tv_usec;
    long end = tve_seq.tv_sec * 1000000 + tve_seq.tv_usec;
    long delta_seq = end - start;

    start = tvs_par.tv_sec * 1000000 + tvs_par.tv_usec;
    end = tve_par.tv_sec * 1000000 + tve_par.tv_usec;
    long delta_par = end - start;

    double acceleration = 1.0 * delta_seq / delta_par;
    double efficiency = acceleration / nbThreads;

    printf("Runtime sequential: %.6f seconds\n", delta_seq / 1000000.0);
    printf("Runtime parallel: %.6f seconds\n", delta_par / 1000000.0);
    printf("Acceleration: %3.6f\n", acceleration);
    printf("Efficiency: %3.6f\n", efficiency);
}
