#include <chrono>
#include <cstring>
#include <thread>

#include <mpi.h>

#include "solver.hpp"
#include "../matrix/matrix.hpp"
#include "../output/output.hpp"

using std::memcpy;

using std::this_thread::sleep_for;
using std::chrono::microseconds;


#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

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
	// idée Pour le gather : mettre en param une matrice égale à à y_range*cols et y copier uniquement ce qui est utile
	//                      ainsi, lors du gather, chaque matrice sera concaténée dans la matrice rows*cols

	// Decoupage horizontal pour envoie de lignes
	int rank, nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

	// Pour les calculs
	double c, l, r, t, b;
	double h_square = h * h;
	double * linePrevBuffer = new double[cols];
	double * lineCurrBuffer = new double[cols];

	// Pour k = 1 à iterations
	// matrice Précédente :
	double ** next_matrix = allocateMatrix(rows, cols);

	int y_range = (rows > nprocs) ? rows/nprocs : 1;    // rows per thread
	int y_reste = rows % nprocs;                        // unallocated_rows

	printf("rank:%d, nprocs:%d,y_range:%d, rows:%d, cols:%d\n", rank, nprocs, y_range,rows,cols);
	int y_begin = rank*y_range;
	int y_end = y_range*(rank+1);


	for (int k = 0; k < iterations; ++k)
	{
		// 1er => sans top
		if (rank == 0)
		{
			//printf("First rank : %d\n", rank);

			// Send derniere col
			MPI_Send(matrix[y_end-1],cols,MPI_DOUBLE, rank+1,0,MPI_COMM_WORLD);            
			// Receive 1 ere ligne
			MPI_Recv(matrix[y_end], cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			
			// Calculs
			memcpy(linePrevBuffer, matrix[0], cols * sizeof(double));
			for(int i = 1; i < y_end; i++) {
				memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));
				for(int j = 1; j < cols-1; j++) {
					c = lineCurrBuffer[j];
					t = linePrevBuffer[j];
					b = matrix[i + 1][j];
					l = lineCurrBuffer[j - 1];
					r = lineCurrBuffer[j + 1];

					sleep_for(microseconds(sleep));
					next_matrix[i][j] = (double) c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
				}
				// Màj de line PrevBuf
				memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
			} 
			// Copy de next_matrix dans matrix
			memcpy(matrix, next_matrix, cols * rows * sizeof(double));            
		}
		// dernier => sans bottom
		else if(rank==nprocs-1)
		{
			/*printf("Last rank : %d, y_begin:%d, y_end:%d\n", rank,y_begin,y_end);
			printf("###################################\n");*/

			// Send 1 ere ligne
			MPI_Send(matrix[y_begin],cols,MPI_DOUBLE, rank-1,0,MPI_COMM_WORLD);
			// Receive derniere ligne
			MPI_Recv(matrix[y_begin-1], cols, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE); 

			// Calculs
			memcpy(linePrevBuffer, matrix[y_begin-1], cols * sizeof(double));
			for(int i = y_begin; i < y_end - 1; i++) {
				memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));

				for(int j = 1; j < cols - 1; j++) {
					c = lineCurrBuffer[j];
					t = linePrevBuffer[j];
					b = matrix[i + 1][j];
					l = lineCurrBuffer[j - 1];
					r = lineCurrBuffer[j + 1];

					sleep_for(microseconds(sleep));
					next_matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
				}
				memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
				// Màj de line PrevBuf
			}

			// Copy de next_matrix dans matrix
			memcpy(matrix, next_matrix, cols * rows * sizeof(double)); 
		}
		else // si nprocs ==2 => pas executé
		{
			/*printf("Between rank : %d, y_begin:%d, y_end:%d\n", rank,y_begin,y_end);
			printf("###################################\n");*/

			// Send 1 ere ligne
			MPI_Send(matrix[y_begin],cols,MPI_DOUBLE, rank-1,0,MPI_COMM_WORLD);
			MPI_Send( matrix[y_end-1],cols,MPI_DOUBLE, rank+1,0,MPI_COMM_WORLD);

			// Receive derniere ligne
			MPI_Recv(matrix[y_begin-1], cols, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);  
			MPI_Recv(matrix[y_end], cols, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);  

			//Calculs
			memcpy(linePrevBuffer, matrix[y_begin-1], cols * sizeof(double));
			for(int i = y_begin; i < y_end ; i++) {
				memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));

				for(int j = 1; j < cols - 1; j++) {
					c = lineCurrBuffer[j];
					t = linePrevBuffer[j];
					b = matrix[i + 1][j];
					l = lineCurrBuffer[j - 1];
					r = lineCurrBuffer[j + 1];

					sleep_for(microseconds(sleep));
					next_matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
				}
				memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
				// Màj de line PrevBuf
			}

			// Copy de next_matrix dans matrix
			//printMatrix(rows, cols, matrix);
			memcpy(matrix, next_matrix, cols * rows * sizeof(double)); 
		}
	}

	printf("RANK :%d\n",rank);
	printMatrix(rows, cols, matrix); 


	if(0 != rank) {
		deallocateMatrix(rows, matrix);
	}

	sleep_for(microseconds(500000));
}
