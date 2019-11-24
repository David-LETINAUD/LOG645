#include <iomanip>
#include <iostream>
#include <chrono>

#include "output.hpp"

using namespace std::chrono;

using std::cout;
using std::endl;
using std::fixed;
using std::flush;
using std::setprecision;
using std::setw;

void printMatrix(int rows, int cols, double ** matrix) {
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			cout << fixed << setw(12) << setprecision(2) << matrix[row][col] << flush;
		}

		cout << endl << flush;
	}

	cout << endl << flush;
}

void printMatrixPartial(int minRow, int maxRow, int minCol, int maxCol, double ** matrix) {
	for (int row = minRow; row < maxRow; row++) {
		for (int col = minCol; col < maxCol; col++) {
			cout << fixed << setw(12) << setprecision(2) << matrix[row][col] << flush;
		}

		cout << endl << flush;
	}

	cout << endl << flush;
}

void printStatistics(long runtime_seq, long runtime_par) {
	double acceleration = 1.0 * runtime_seq / runtime_par;

	cout << "Runtime sequential: " << runtime_seq / 1000000.0 << " seconds" << endl << flush;
	cout << "Runtime parallel  : " << runtime_par / 1000000.0 << " seconds" << endl << flush;
	cout << "Acceleration      : " << acceleration << endl << flush;
}