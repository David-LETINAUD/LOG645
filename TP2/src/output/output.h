#ifndef OUTPUT_H
#define OUTPUT_H

void printMatrix(int rows, int cols, int ** matrix);
void printStatistics(int nbThreads, struct timeval tvs_seq, struct timeval tve_seq, struct timeval tvs_par, struct timeval tve_par);

#endif
