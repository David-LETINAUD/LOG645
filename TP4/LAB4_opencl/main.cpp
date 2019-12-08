#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <string>

#include "matrix.hpp"
#include "output.hpp"
#include "sequential.hpp"
#include "parallel.hpp"

void usage();
void command(int argc, char* argv[]);

void initial(int rows, int cols, double ** matrix);
long sequential(int rows, int cols, int iters, double td, double h, double ** matrix);
long parallel(int rows, int cols, int iters, double td, double h, double ** matrix, const char * kernelFileName);

using namespace std::chrono;

using std::cout;
using std::endl;
using std::flush;
using std::setprecision;
using std::setw;

int main(int argc, char* argv[]) {
	// Arguments.
	int rows;
	int cols;
	int iters;
	int minDisplayRow;
	int maxDisplayRow;
	int minDisplayCol;
	int maxDisplayCol;
	double td;
	double h;
	char * kernelFileName;

	// Timing variables.
	long runtime_seq = 0;
	long runtime_par = 0;

	if (11 != argc) {
		usage();
		return EXIT_FAILURE;
	}

	rows = atoi(argv[1]);
	cols = atoi(argv[2]);
	iters = atoi(argv[3]);
	td = atof(argv[4]);
	h = atof(argv[5]);
	minDisplayRow = atoi(argv[6]);
	maxDisplayRow = atoi(argv[7]);
	minDisplayCol = atoi(argv[8]);
	maxDisplayCol = atoi(argv[9]);
	kernelFileName = argv[10];

	command(argc, argv);
	double ** matrix = allocateMatrix(rows, cols);

	cout << "-----  INITIAL   -----" << endl << flush;
	initial(rows, cols, matrix);
	//printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);
	//printMatrix(rows, cols, matrix);
	printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);

	cout << "----- SEQUENTIAL -----" << endl << flush;
	runtime_seq = sequential(rows, cols, iters, td, h, matrix);
	//printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);
	//printMatrix(rows, cols, matrix);
	printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);

	cout << "-----  PARALLEL  -----" << endl << flush;
	runtime_par = parallel(rows, cols, iters, td, h, matrix, kernelFileName);
	printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);
	//printMatrix(rows, cols, matrix);

	printStatistics(runtime_seq, runtime_par);

	deallocateMatrix(rows, matrix);

	system("pause");
	return EXIT_SUCCESS;
}

void usage() {
	cout << "Invalid arguments." << endl << flush;
	cout << "Arguments: m n np td h minDisplayRow maxDisplayRow minDisplayCol maxDisplayCol kernelFileName" << endl << flush;
}

void command(int argc, char* argv[]) {
	cout << "Command:" << flush;

	for (int i = 0; i < argc; i++) {
		cout << " " << argv[i] << flush;
	}

	cout << endl << flush;
}

void initial(int rows, int cols, double ** matrix) {
	fillMatrix(rows, cols, matrix);
}

long sequential(int rows, int cols, int iters, double td, double h, double ** matrix) {
	fillMatrix(rows, cols, matrix);

	time_point<high_resolution_clock> timepoint_s = high_resolution_clock::now();
	solveSeq(rows, cols, iters, td, h, matrix);
	time_point<high_resolution_clock> timepoint_e = high_resolution_clock::now();

	return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}

long parallel(int rows, int cols, int iters, double td, double h, double ** matrix, const char * kernelFileName) {
	fillMatrix(rows, cols, matrix);
	const int matrix_size = (const int)rows * (const int)cols;

	double* initial_matrix = (double*)malloc(matrix_size * sizeof(double));
	double* final_matrix = (double*)malloc(matrix_size * sizeof(double));

	convert_to_1d_matrix(rows, cols, matrix, initial_matrix);
	
	/*int pause;
	std::cin >> pause;*/
	time_point<high_resolution_clock> timepoint_s = high_resolution_clock::now();
	solvePar(rows, cols, iters, td, h, initial_matrix, final_matrix, kernelFileName);
	time_point<high_resolution_clock> timepoint_e = high_resolution_clock::now();

	convert_to_2d_matrix(rows, cols, final_matrix, matrix);

	//std::cin >> pause;

	return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}