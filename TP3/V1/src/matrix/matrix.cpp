#include "matrix.hpp"

#include <stdio.h>

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
		matrix[i] = nullptr;
	}

	delete(matrix);
	*matrix = nullptr;
}

void fillMatrix(int rows, int cols, double ** matrix) {
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
			matrix[row][col] = row * (rows - row - 1) * col * (cols - col - 1);
		}
	}
}

// Fonction ajoutée pour renverser la matrice
double ** flipMatrix(int rows, int cols, double ** matrix) {
	double ** matrix_flipped = allocateMatrix(cols, rows);
	for (int i = 0; i < cols; i++) {
		for (int j = 0; j < rows; j++) {
			matrix_flipped[i][j] = matrix[j][i];
		}
	}
	return matrix_flipped;
}
