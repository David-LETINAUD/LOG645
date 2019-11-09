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

int main (int argc, char* argv[]) {
	int rows = 39;
	int cols = 10;
	
	int mpi_status = MPI_Init(&argc, &argv);
    if(MPI_SUCCESS != mpi_status) {
        cout << "MPI initialization failure." << endl << flush;
        return EXIT_FAILURE;
    }
	
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
	cout << "Thread id " << rank << ": " << indexes << " indexes"; 
}
