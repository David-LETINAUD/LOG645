#ifndef PARALLEL_HPP
#define PARALLEL_HPP

void solvePar(int rows, int cols, int iterations, double td, double h, double* initial_matrix, double* final_matrix, const char* kernelFileName);

#endif