#ifndef MATRIX_HPP
#define MATRIX_HPP

double ** allocateMatrix(int rows, int cols);
void deallocateMatrix(int rows, double ** matrix);

void fillMatrix(int rows, int cols, double ** matrix);
void convert_to_1d_matrix(int rows, int cols, double** matrix_init, double* matrix_1d);
void convert_to_2d_matrix(int rows, int cols, double* matrix_init, double** matrix_2d);

#endif