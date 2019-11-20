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

// Définition des tags de contrôle pour l'envoi de certains messages MPI
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
	// pour chaque processus
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
    
	// Si le nombre de processus alloué est supérieur au nombre de
	// lignes à calculer, n'utiliser qu'une ligne par processus et
	// quitter l'exécution de la fonction sur les processus qui n'auront
	// pas de lignes attribuées.
    if (nprocs > rows) nprocs = rows;
    if (rank >= rows) return;

    for (int k = 0; k < iterations; ++k)
    {
        // Le premier processus n'a aucun processus voisin en haut de
		// lui, on envoie seulement les messages au voisin en dessous.
        if (rank == 0)
        {
            // Envoyer la dernière ligne gérée par le processus
            MPI_Send(matrix[y_end-1],cols,MPI_DOUBLE, rank + 1, k, MPI_COMM_WORLD);            
            // Recevoir la première ligne du processus voisin (dessous)
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
                // Mise à jour de linePrevBuf
                memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
            } 
        }
        // Le dernier processus n'a aucun voisin en dessous de lui, on
		// envoie seulement les messages au voisin du dessus.
        else if (rank == nprocs - 1)
        {
            // Envoyer la première ligne gérée par le processus
            MPI_Send(matrix[y_begin],cols,MPI_DOUBLE, rank-1, k, MPI_COMM_WORLD);
            // Recevoir la dernière ligne du processus voisin (dessus)
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
        // Les processus se trouvant entre le premier et le dernier
        // processus ont des voisins au dessus et en dessous d'eux, donc
        // ils doivent envoyer leurs premières lignes et leurs dernières
        // lignes.
        else
        {
            // Envoyer la première ligne gérée par le processus au
			// processus voisin au dessus
            MPI_Send(matrix[y_begin],cols,MPI_DOUBLE, rank-1,k,MPI_COMM_WORLD);
			// Envoyer la dernière ligne gérée par le processus au
			// processus voisin en dessous
            MPI_Send( matrix[y_end-1],cols,MPI_DOUBLE, rank+1,k,MPI_COMM_WORLD);

            // Recevoir la première ligne du processus voisin en dessous
            // et la dernière ligne du processus voisin au dessus
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

	// Le premier processus reçoit les données calculées par tous les
	// autres processus
    if(rank == 0) {
        for (int i = 1; i < nprocs; i++) {
			double * recv_buffer = new double[cols];
			
			// On reçoit les index correspondant à la plage de lignes
			// de laquelle le processus est responsable 
			int start_index, end_index;
			MPI_Recv(&start_index, 1, MPI_INT, i, S_INDEX_CONTROL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&end_index, 1, MPI_INT, i, S_INDEX_CONTROL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			
			// On reçoit les lignes calculées par le processus et on les
			// place dans la matrice
			for (int j = start_index; j < end_index; j++) {
				MPI_Recv(recv_buffer, cols, MPI_DOUBLE, i, S_MATRIX_XFER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				memcpy(matrix[j], recv_buffer, cols * sizeof(double));
			}
		}
    }
	// Les processus envoient leur données au premier processus
    else {
		// Envoi des index correspondant à la plage de lignes de
		// laquelle le processus est responsable
		MPI_Send(&y_begin, 1, MPI_INT, 0, S_INDEX_CONTROL, MPI_COMM_WORLD);
		MPI_Send(&y_end, 1, MPI_INT, 0, S_INDEX_CONTROL, MPI_COMM_WORLD);
		
		// On envoie les lignes calculées par le processus courant au
		// premier processus
		for (int j = y_begin; j < y_end; j++) {
			double * current_column = matrix[j];
			MPI_Send(current_column, cols, MPI_DOUBLE, 0, S_MATRIX_XFER, MPI_COMM_WORLD);
		}
	}

    sleep_for(microseconds(500000));
}
