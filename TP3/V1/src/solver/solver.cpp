#include <chrono>
#include <cstring>
#include <thread>
#include <mpi.h>

#include "solver.hpp"
#include "../matrix/matrix.hpp"

using std::memcpy;

using std::this_thread::sleep_for;
using std::chrono::microseconds;

void solveSeq(int rows, int cols, int iterations, double td, double h, int sleep, double ** matrix) {
    double c, l, r, t, b;
    
    double h_square = h * h;

    double * linePrevBuffer = new double[cols];
    double * lineCurrBuffer = new double[cols];

    for(int k = 0; k < iterations; k++) {

        memcpy(linePrevBuffer, matrix[0], cols * sizeof(double));
        for(int i = 1; i < rows - 1; i++) {

            memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));
            for(int j = 1; j < cols - 1; j++) {
                c = lineCurrBuffer[j];
                t = linePrevBuffer[j];
                b = matrix[i + 1][j];
                l = lineCurrBuffer[j - 1];
                r = lineCurrBuffer[j + 1];


                sleep_for(microseconds(sleep));
                matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
            }

            memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
        }
    }
}

void solvePar(int rows, int cols, int iterations, double td, double h, int sleep, double ** matrix) {

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Récupration du nombre de threads
    int number_of_threads;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_threads);
    
	if (rank == 0) {
		// On calcule le nombre de lignes à distribuer à chacun des
		// threads disponibles.
		
		int rows_per_thread;
		if (rows <= number_of_threads) rows_per_thread = 1;
		else rows_per_thread = rows / number_of_threads;
		
		int unallocated_rows = rows % number_of_threads;
		
		int* indexes_per_thread = new int[number_of_threads];
		
		for (int i = 0; i < number_of_threads; i++) {
			int indexes = rows_per_thread;
			if (unallocated_rows > 0) {
				unallocated_rows--;
				indexes++;
			}
			MPI_Send(&indexes, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	}
	
	int indexes;
	MPI_Recv(&indexes, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	
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


    if(0 != rank) {
        deallocateMatrix(rows, matrix);
    }

    sleep_for(microseconds(500000));
}