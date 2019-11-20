#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

#include <mpi.h>

#include "matrix/matrix.hpp"
#include "output/output.hpp"
#include "solver/solver.hpp"

#include <chrono>
using std::this_thread::sleep_for;

void usage();
void command(int argc, char* argv[]);

void initial(int rows, int cols);
long sequential(int rows, int cols, int iters, double td, double h, int sleep);
long parallel(int rows, int cols, int iters, double td, double h, int sleep);

using namespace std::chrono;

using std::cout;
using std::endl;
using std::flush;
using std::setprecision;
using std::setw;
using std::stod;
using std::stoi;

int main(int argc, char* argv[]) {
	// Arguments
	int rows;
	int cols;
	int iters;
	double td;
	double h;

	// MPI variables
	int mpi_status;
	int rank, nprocs;

	// Resolution variables
	// Sleep will be in microseconds during execution.
	int sleep = 1;

	// Timing variables
	long runtime_seq = 0;
	long runtime_par = 0;

	if(6 != argc) {
		usage();
		return EXIT_FAILURE;
	}

	mpi_status = MPI_Init(&argc, &argv);
	if(MPI_SUCCESS != mpi_status) {
		cout << "MPI initialization failure." << endl << flush;
		return EXIT_FAILURE;
	}

	rows = stoi(argv[1], nullptr, 10);
	cols = stoi(argv[2], nullptr, 10);
	iters = stoi(argv[3], nullptr, 10);
	td = stod(argv[4], nullptr);
	h = stod(argv[5], nullptr);

	// Get the process rank and number of process
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs); 

	// First process run the sequential program
	if(0 == rank) {
		command(argc, argv);
		initial(rows, cols);
		runtime_seq = sequential(rows, cols, iters, td, h, sleep);
	}

	// Ensure that no process will start computing early.
	MPI_Barrier(MPI_COMM_WORLD);

	// Run the parallel program
	runtime_par = parallel(rows, cols, iters, td, h, sleep);

	if(0 == rank) {
		printStatistics(nprocs, runtime_seq, runtime_par);
	}

	mpi_status = MPI_Finalize();
	if(MPI_SUCCESS != mpi_status) {
		cout << "Execution finalization terminated in error." << endl << flush;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void usage() {
	cout << "Invalid arguments." << endl << flush;
	cout << "Arguments: m n np td h" << endl << flush;
}

void command(int argc, char* argv[]) {
	cout << "Command:" << flush;

	for(int i = 0; i < argc; i++) {
		cout << " " << argv[i] << flush;
	}

	cout << endl << flush;
}

void initial(int rows, int cols) {
	double ** matrix = allocateMatrix(rows, cols);
	fillMatrix(rows, cols, matrix);

	cout << "-----  INITIAL   -----" << endl << flush;
	printMatrix(rows, cols, matrix);

	deallocateMatrix(rows, matrix);
}

long sequential(int rows, int cols, int iters, double td, double h, int sleep) {
	double ** matrix = allocateMatrix(rows, cols);
	fillMatrix(rows, cols, matrix);

	time_point<high_resolution_clock> timepoint_s = high_resolution_clock::now();
	solveSeq(rows, cols, iters, td, h, sleep, matrix);
	time_point<high_resolution_clock> timepoint_e = high_resolution_clock::now();

	cout << "----- SEQUENTIAL -----" << endl << flush;
	printMatrix(rows, cols, matrix);

	deallocateMatrix(rows, matrix);
	return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}

long parallel(int rows, int cols, int iters, double td, double h, int sleep) {
	
	// On souhaite paralléliser sur le nombre de lignes.
	// Si le nombre de colonnes est plus grand que le nombre de lignes,
	// on renverse la matrice (on la re-renversera plus tard)
	bool matrix_flipped = cols > rows;
	double ** matrix;
	
	if (matrix_flipped) {
		matrix = allocateMatrix(cols, rows);
		fillMatrix(cols, rows, matrix);
	} else {
		matrix = allocateMatrix(rows, cols);
		fillMatrix(rows, cols, matrix);
	}
	
	// Calcul du temps d'exécution des algorithmes
	time_point<high_resolution_clock> timepoint_s, timepoint_e;
	if (matrix_flipped) {
		timepoint_s = high_resolution_clock::now();
		solvePar(cols, rows, iters, td, h, sleep, matrix);
		timepoint_e = high_resolution_clock::now();
	} else {
		timepoint_s = high_resolution_clock::now();
		solvePar(rows, cols, iters, td, h, sleep, matrix);
		timepoint_e = high_resolution_clock::now();
	}
	
	// Get the process rank and number of process
	int rank, nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	
	// The first process print the result and deallocate the matrix
	if (rank == 0) {
		cout << "-----  PARALLEL -----" << endl << flush;
		
		// Redresser la matrice si elle a été préalablement renversée
		if (matrix_flipped) {
			double ** intermediary_matrix = flipMatrix(cols, rows, matrix);
			printMatrix(rows, cols, intermediary_matrix);
			
			deallocateMatrix(rows, intermediary_matrix);
			deallocateMatrix(cols, matrix);
		}
		else {
			printMatrix(rows, cols, matrix);
			deallocateMatrix(rows, matrix);
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}
