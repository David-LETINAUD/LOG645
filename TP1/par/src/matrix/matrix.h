#ifndef MATRIX_H
#define MATRIX_H

int ** allocateMatrix(int dim);
void deallocateMatrix(int dim, int * matrix);

void fillMatrix(int dim, int initialValue, int * matrix);

#endif
