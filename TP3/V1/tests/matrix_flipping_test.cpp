#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

#include <mpi.h>

#include "../src/matrix/matrix.hpp"
#include "../src/output/output.hpp"
#include "../src/solver/solver.hpp"

using namespace std::chrono;

using std::cout;
using std::endl;
using std::flush;
using std::setprecision;
using std::setw;
using std::stod;
using std::stoi;

int main () {
	int rows = 5;
	int cols = 10;

	bool matrix_flipped = cols > rows;
	double ** matrix;
	
	// Création d'une matrice de travail
	// (au cas où nous avons besoin de renverser la matrice)
	double ** work_matrix;
	if (matrix_flipped) {
		work_matrix = allocateMatrix(cols, rows);
		fillMatrix(cols, rows, work_matrix);
	} else {
		work_matrix = allocateMatrix(rows, cols);
		fillMatrix(rows, cols, work_matrix);
	}

	// Renversage de la matrice de travail si nécessaire
	matrix = (matrix_flipped ? flipMatrix(cols, rows, work_matrix) : work_matrix);
	
	if(nullptr != *matrix) {
		cout << "-----  PARALLEL  -----" << endl << flush;
		printMatrix(rows, cols, matrix);
		deallocateMatrix(rows, matrix);
	}
}
