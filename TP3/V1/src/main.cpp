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
	// Arguments.
	int rows;
	int cols;
	int iters;
	double td;
	double h;

	// MPI variables.
	int mpi_status;
	int rank, nprocs;

	// Resolution variables.
	// Sleep will be in microseconds during execution.
	int sleep = 1;

	// Timing variables.
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

	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs); 

	if(0 == rank) {
		command(argc, argv);
		initial(rows, cols);
		runtime_seq = sequential(rows, cols, iters, td, h, sleep);
	}

	//printf("MPI_Barrier\n");
	// Ensure that no process will start computing early.
	MPI_Barrier(MPI_COMM_WORLD);

	//printf("runtime_par\n");
	runtime_par = parallel(rows, cols, iters, td, h, sleep);

	if(0 == rank) {
		printStatistics(1, runtime_seq, runtime_par);
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
	// Si le nombre de colonnes est plus long que le nombre de lignes,
	// on renverse la matrice (on la re-renversera plus tard)
	
	bool matrix_flipped = cols > rows;

	double ** matrix;// = allocateMatrix(rows, cols);
	//fillMatrix(rows, cols, matrix);
	
	// Création d'une matrice de travail
	// (au cas où nous avons besoin de renverser la matrice)
	//double ** work_matrix;
	if (matrix_flipped) {
		matrix = allocateMatrix(cols, rows);
		fillMatrix(cols, rows, matrix);
	} else {
		matrix = allocateMatrix(rows, cols);
		fillMatrix(rows, cols, matrix);
	}
	
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
	
	// Renversage de la matrice de travail si nécessaire
	//matrix = (matrix_flipped ? flipMatrix(cols, rows, work_matrix) : work_matrix);
	/*if (matrix_flipped) {
		cout << "Matrix was flipped\n";
		double ** fuck_sake;
		fuck_sake = flipMatrix(cols, rows, work_matrix);
		memcpy(matrix, fuck_sake, cols * rows * sizeof(double *));
		deallocateMatrix(rows, fuck_sake);
		deallocateMatrix(cols, work_matrix);
	} else {
		cout << "Matrix wasn't flipped\n";
		memcpy(matrix, work_matrix, rows * cols * sizeof(double));
		deallocateMatrix(rows, work_matrix);
	}*/
	//memcpy(matrix, (matrix_flipped ? flipMatrix(cols, rows, work_matrix) : work_matrix), rows * cols * sizeof(double *));
	//deallocateMatrix((matrix_flipped ? cols : rows), work_matrix);
	
	// A faire ! recuperation des matrices
    //MPI_Gather( sendarray, rows * cols, MPI_DOUBLE, matrix, 100, MPI_INT, root, comm);
	
	int rank, nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	
	/*if(nullptr != *matrix) {
		int current_rank = 0;
		while (current_rank < nprocs) {
			if (rank == current_rank) {
				cout << "-----  PARALLEL " << rank << " -----" << endl << flush;
				printMatrix(rows, cols, matrix);
			}
			current_rank++;
			MPI_Barrier(MPI_COMM_WORLD);
		}
		deallocateMatrix(rows, matrix);
	}*/
	
	if (rank == 0) {
		cout << "-----  PARALLEL " << rank << " -----" << endl << flush;
		//cout << "Rows: " << rows << " Cols: " << cols << endl<< flush;
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
	

	// Calcul des index dont chaque tache doit s'occuper

	// Pour k=1 à <=iters k++
		//partie du dessus
			// send dernière ligne 
			// calcul
		// partie du dessous
			// send premiere ligne 
			// calcul

		// Autres parties
			// recevoir message partie du dessus
			// recevoir message partie du dessous
			// calcul
			// send premiere ligne 
			// send deniere ligne 


	MPI_Barrier(MPI_COMM_WORLD);
	return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}
