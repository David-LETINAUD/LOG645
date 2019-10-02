#ifndef MATRIX_H
#define MATRIX_H

int ** allocateMatrix(int rows, int cols);
void deallocateMatrix(int rows, int ** matrix);

void fillMatrix(int rows, int cols, int initialValue, int ** matrix);

#endif
