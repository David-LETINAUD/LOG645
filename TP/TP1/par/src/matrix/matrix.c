#include <stdlib.h>

int* allocateMatrix(int dim) {
	return malloc(dim * sizeof(int *));
}

void deallocateMatrix(int rows, int * matrix) {
	free(matrix);
}

void fillMatrix(int dim, int initialValue, int * matrix) {
    for(int i = 0; i < dim; ++i) {
    	matrix[i] = initialValue;
    }
}
