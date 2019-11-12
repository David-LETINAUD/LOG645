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
	int rows = 19;
	int cols = 10;
	
	int mpi_status = MPI_Init(&argc, &argv);
    if(MPI_SUCCESS != mpi_status) {
        cout << "MPI initialization failure." << endl << flush;
        return EXIT_FAILURE;
    }
	
	int rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int number_of_threads = 4;
    //MPI_Comm_size(MPI_COMM_WORLD, &number_of_threads);
	
	
	for (rank = 0; rank < 4; rank++) {
		int rows_per_thread;
		int extra_rows;
		
		if (rows <= number_of_threads) {
			rows_per_thread = 1;
			extra_rows = 0;
		}
		else {
			rows_per_thread = rows / number_of_threads;
			extra_rows = rows % number_of_threads;
		}
		
		int number_of_indexes = rows_per_thread;
		if (extra_rows - rank > 0) number_of_indexes++;
		
		//Index sur lequel démarrer:
		int start_index = (rank * rows_per_thread) + (extra_rows - rank > 0 ? rank : extra_rows);
		int end_index = start_index + number_of_indexes;
		cout << "Thread id " << rank << ": Starts at " << start_index << ", ends at " << end_index << "\n"; 
	}
	
}