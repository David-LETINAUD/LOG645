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
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    int y_range = (int) cols/nprocs ;
    printf("rank:%d, nprocs:%d,y_range:%d, rows:%d, cols:%d\n", rank, nprocs, y_range,rows,cols);

    // Pour les calculs
    double c, l, r, t, b;
    double h_square = h * h;
    double * linePrevBuffer = new double[cols];
    double * lineCurrBuffer = new double[cols];

// Pour k = 1 à iterations
// matrice Précédente :
double ** next_matrix;
next_matrix = allocateMatrix(cols, rows);
    for (int k = 0; k < iterations; ++k)
    {
        // 1er => sans top
        if (rank == 0)
        {
            printf("First rank : %d\n", rank);

            // Send derniere col
            // /!\ MPI_DOUBLE sinon reception de la moitier des données!!!!
            MPI_Send(&matrix[y_range][0],rows,MPI_DOUBLE, rank+1,0,MPI_COMM_WORLD);
            for(int i=0 ; i< cols ; ++i)
            {
                printf("send%f ",matrix[y_range][i]);
            }
            printf("\n");
            // Receive 1 ere ligne
            MPI_Recv(&matrix[y_range+1][0], rows, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            /*for(int i=0 ; i< cols ; ++i)
            {
                printf("%f ",matrix[y_range+1][i]);
            }
            printf("\n");*/

            // Calculs
            memcpy(linePrevBuffer, matrix[0], rows * sizeof(double));
            for(int i = 1; i < cols - 1; i++) {
                memcpy(lineCurrBuffer, matrix[i], rows * sizeof(double));
                for(int j = 1; j < y_range -1; j++) {
                    c = lineCurrBuffer[j];
                    t = linePrevBuffer[j];
                    b = matrix[i + 1][j];
                    l = lineCurrBuffer[j - 1];
                    r = lineCurrBuffer[j + 1];

                    sleep_for(microseconds(sleep));
                    next_matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
                // Màj de line PrevBuf
                memcpy(linePrevBuffer, lineCurrBuffer, rows * sizeof(double));
            } 
            // Copy de next_matrix dans matrix
            printMatrix(rows, cols, matrix);
            memcpy(matrix, next_matrix, cols * rows * sizeof(double));
            //printMatrix(rows, cols, matrix);
        }
        // dernier => sans bottom
        else if(rank==nprocs-1)
        {
            int y_begin = rank*y_range;
            int y_end = y_range*(rank+1);
            printf("Last rank : %d\n", rank);
            printf("###################################\n");

            // Send 1 ere ligne
            MPI_Send(&matrix[y_begin][0],rows,MPI_DOUBLE, rank-1,0,MPI_COMM_WORLD);
            // Receive derniere ligne
            MPI_Recv(linePrevBuffer, rows, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);           
            for(int i=0 ; i< cols ; ++i)
            {
                printf("recv%f ",linePrevBuffer[i]);
            }
            printf("\n");
            

            // Calculs
            //memcpy(linePrevBuffer, matrix[y_begin], cols * sizeof(double));
            for(int i = 1; i < cols - 1; i++) {
                memcpy(lineCurrBuffer, matrix[i], rows * sizeof(double));
                for(int j = y_begin; j < y_end - 1; j++) {
                    c = lineCurrBuffer[j];
                    t = linePrevBuffer[j];
                    b = matrix[i + 1][j];
                    l = lineCurrBuffer[j - 1];
                    r = lineCurrBuffer[j + 1];

                    sleep_for(microseconds(sleep));
                    next_matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
                // Màj de line PrevBuf
                memcpy(linePrevBuffer, lineCurrBuffer, rows * sizeof(double));
            }

            // Copy de next_matrix dans matrix
            //printMatrix(rows, cols, matrix);
            memcpy(matrix, next_matrix, cols * rows * sizeof(double)); 

            printMatrix(rows, cols, matrix);
        }
        else // si nprocs ==2 => pas executé
        {
            printf("Between rank : %d\n", rank);

            // Send derniere ligne
            // Send 1 ere ligne
            // Receive derniere ligne
            // Receive 1 ere ligne

            // Calculs
            memcpy(linePrevBuffer, matrix[0], cols * sizeof(double));
            for(int i = 1; i < rows - 1; i++) {
                memcpy(lineCurrBuffer, matrix[i], cols * sizeof(double));
                for(int j = rank*y_range; j < y_range*(rank+1); j++) {
                    c = lineCurrBuffer[j];
                    t = linePrevBuffer[j];
                    b = matrix[i + 1][j];
                    l = lineCurrBuffer[j - 1];
                    r = lineCurrBuffer[j + 1];

                    sleep_for(microseconds(sleep));
                    matrix[i][j] = c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
                }
                // Màj de line PrevBuf
                memcpy(linePrevBuffer, lineCurrBuffer, cols * sizeof(double));
            } 
            printMatrix(rows, cols, matrix);
            
        }
    }


    if(0 != rank) {
        deallocateMatrix(rows, matrix);
    }

    sleep_for(microseconds(500000));
}
