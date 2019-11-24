#ifndef OUTPUT_HPP
#define OUTPUT_HPP

void printMatrix(int rows, int cols, double ** matrix);
void printMatrixPartial(int minRow, int maxRow, int minCol, int maxCol, double ** matrix);
void printStatistics(long runtime_seq, long runtime_par);

#endif