#include <chrono>
#include <cstring>
#include <thread>
#include <iostream>

#include <mpi.h>

#include "solver.hpp"
#include "../matrix/matrix.hpp"
#include "../output/output.hpp"

using std::memcpy;

using std::this_thread::sleep_for;
using std::chrono::microseconds;
using std::cout;


#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

#define S_INDEX_CONTROL 10000
#define S_MATRIX_XFER   10001

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
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    // Pour les calculs
    double c, l, r, t, b;
    double h_square = h * h;
    double * linePrevBuffer = new double[cols];
    double * lineCurrBuffer = new double[cols];

	// Calculs des index des lignes à prendre en charge
    int y_range = (rows > nprocs) ? rows/nprocs : 1;
    int y_reste = (rows > nprocs) ? rows % nprocs : 0;

    int y_begin = rank * y_range ;
    int y_end = y_range*(rank+1) ;

    if (y_reste)
    {
        if(rank<y_reste)
        {
            y_begin += rank ;
            y_end += (rank+1) ;
        }
        else
        {
            y_begin+=y_reste;
            y_end+=y_reste;
        }
    }
    
	// Si le nombre de processus alloué est supérieur.. 
	// ..au nombre de lignes à calculer alors :
	// 	  => Utiliser 1 ligne par processus
    if (nprocs > rows) nprocs = rows;
	//    => Ne rien faire avec les autres processus
    if (rank >= rows) return;

    for (int k = 0; k < iterations; ++k)
    {
        // 1er => sans voisin du haut
        if (rank == 0)
        {
            // Send last row
            MPI_Send(matrix[y_end-1],cols,MPI_DOUBLE, rank + 1, k, MPI_COMM_WORLD);            
            // Receive missing row
            MPI_Recv(matrix[y_end], cols, MPI_DOUBLE, rank + 1, k, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            
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
                    matrix[i][j] = (double) c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
                // Màj de line PrevBuf
                memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
            } 
        }
        // dernier => sans voisin du bas
        else if(rank==nprocs-1)
        {
            // Send first row
            MPI_Send(matrix[y_begin],cols,MPI_DOUBLE, rank-1, k, MPI_COMM_WORLD);
            // Receive missing row
            MPI_Recv(matrix[y_begin-1], cols, MPI_DOUBLE, rank-1, k, MPI_COMM_WORLD,MPI_STATUS_IGNORE); 

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
                    matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
                memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
            }
        }
        else // processus du milieu : si nprocs == 2 => pas executé
        {
            // Send first row
            MPI_Send(matrix[y_begin],cols,MPI_DOUBLE, rank-1,k,MPI_COMM_WORLD);
			// Send last row
            MPI_Send( matrix[y_end-1],cols,MPI_DOUBLE, rank+1,k,MPI_COMM_WORLD);

            // Receive missing rows
            MPI_Recv(matrix[y_begin-1], cols, MPI_DOUBLE, rank-1, k, MPI_COMM_WORLD,MPI_STATUS_IGNORE); 		
            MPI_Recv(matrix[y_end], cols, MPI_DOUBLE, rank+1, k, MPI_COMM_WORLD,MPI_STATUS_IGNORE);  

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
                    matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
                memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
            }
        }
    }

	// Le 1er processus recoit les données calculées par tous les autres processus
    if(rank == 0) {
        for (int i = 1; i < nprocs; i++) {
			double * recv_buffer = new double[cols];
			
			int start_index, end_index;
			MPI_Recv(&start_index, 1, MPI_INT, i, S_INDEX_CONTROL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&end_index, 1, MPI_INT, i, S_INDEX_CONTROL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for (int j = start_index; j < end_index; j++) {
				MPI_Recv(recv_buffer, cols, MPI_DOUBLE, i, S_MATRIX_XFER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				memcpy(matrix[j], recv_buffer, cols * sizeof(double));
			}
		}
    }
	// Les processus envoient leur données au 1er processus
    else {
		MPI_Send(&y_begin, 1, MPI_INT, 0, S_INDEX_CONTROL, MPI_COMM_WORLD);
		MPI_Send(&y_end, 1, MPI_INT, 0, S_INDEX_CONTROL, MPI_COMM_WORLD);
		for (int j = y_begin; j < y_end; j++) {
			double * current_column = matrix[j];
			MPI_Send(current_column, cols, MPI_DOUBLE, 0, S_MATRIX_XFER, MPI_COMM_WORLD);
		}
	}

    sleep_for(microseconds(500000));
}
