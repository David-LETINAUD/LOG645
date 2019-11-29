#include "matrix.hpp"

double ** allocateMatrix(int rows, int cols) {
    double ** matrix = new double*[rows];

    for(int i = 0; i < rows; i++) {
        matrix[i] = new double[cols];
    }

    return matrix;
}

void deallocateMatrix(int rows, double ** matrix) {
    for(int i = 0; i < rows; i++) {
        delete(matrix[i]);
    }

    delete(matrix);
}

void fillMatrix(int rows, int cols, double ** matrix) {
     for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            matrix[row][col] = row * (rows - row - 1) * col * (cols - col - 1);
        }
    }
}

void convert_to_1d_matrix(int rows, int cols, double** matrix_init, double* matrix_1d) {
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			matrix_1d[y * cols + x] = matrix_init[y][x];
		}
	}
}

void convert_to_2d_matrix(int rows, int cols, double* matrix_init, double** matrix_2d) {
	for (int i = 0; i < rows * cols; i++) {
		int x = i % cols;
		int y = i / cols;
		matrix_2d[y][x] = matrix_init[i];
	}
}