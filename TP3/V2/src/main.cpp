#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <mpi.h>

// Constants
#define MASTER 0
#define NO_NEIGH -1
#define BEGIN_TAG 100
#define TRANSMIT_LEFT_TAG 101
#define TRANSMIT_RIGHT_TAG 102
#define END_TAG 103
#define VERTICAL 0
#define HORIZONTAL 1
#define MODE_COMPLET 0
#define MODE_PAR 1
#define MODE_NON_VERBOSE 1

// Global variables
int size, id, n, m, np, procs, matrixsize, direction, mode;
double td, h;

// Functions declarations
void executeseq(int n, int m, int np, double matrix[2][n][m]);
void executepar(int n, int m, int np, double matrix[2][n][m]);
void initmatrix(int rows, int columns, double matrix[rows][columns]);
void zeromatrix(int rows, int columns, int iterations, double matrix[iterations][rows][columns]);
void printmatrix(int rows, int columns, double matrix[rows][columns]);
double getcurrenttime();

int main(int argc, char* argv[])
{
    double start, end, timeseq, timepar;
    MPI_Init(&argc, &argv);
    
    // Validate parameters
    if (argc == 7)
    {
        n     = atoi(argv[1]);
        m     = atoi(argv[2]);
        np    = atoi(argv[3]);
        td    = atof(argv[4]);
        h     = atof(argv[5]);
        procs = atoi(argv[6]);
        mode  = 0;
    }
    else if (argc == 8)
    {
        n     = atoi(argv[1]);
        m     = atoi(argv[2]);
        np    = atoi(argv[3]);
        td    = atof(argv[4]);
        h     = atof(argv[5]);
        procs = atoi(argv[6]);
        mode  = atoi(argv[7]);
    }
    else
    {
        printf("Il manque des arguments, quitting.\n");
        return EXIT_FAILURE;
    }
    
    // Retrieve the actual rank (id) and the numbers of procs
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (id == MASTER) {
        double matrix[2][n][m];
        zeromatrix(n, m, 2, matrix);
        
        if (mode != MODE_PAR) {
            if (mode == MODE_COMPLET) printf("Version sequentiel\n");
            // Initialize la matrice
            initmatrix(n, m, matrix[0]);
            if (mode == MODE_COMPLET) printf("initiale\n====\n");
            // Affiche la matrice initial
            if (mode == MODE_COMPLET) printmatrix(n, m, matrix[0]);
            // Demarrer le timer
            start = getcurrenttime();
            // Traitement  sequentiel
            executeseq(n, m, np, matrix);
            // Arreter le timer
            end = getcurrenttime();
            timeseq = end - start;
            // Affiche la matrice initial
            if (mode == MODE_COMPLET) printf("final\n=====\n");
            if (mode == MODE_COMPLET) printmatrix(n, m, matrix[np%2]);
        }
        
        
        if (mode == MODE_COMPLET) printf("\n\n");
        if (mode == MODE_COMPLET) printf("Version parallele\n");
        // Initialize la matrice
        initmatrix(n, m, matrix[0]);
        if (mode == MODE_COMPLET) printf("initiale\n====\n");
        // Affiche la matrice initial
        if (mode == MODE_COMPLET) printmatrix(n, m, matrix[0]);
        // Demarrer le timer
        start = getcurrenttime();
        // Traitement  sequentiel
        executepar(n, m, np, matrix);
        // Arreter le timer
        end = getcurrenttime();
        timepar = end - start;
        // Affiche la matrice final
        if (mode == MODE_COMPLET) printf("final\n=====\n");
        if (mode == MODE_COMPLET) printmatrix(n, m, matrix[0]);
        
        if (mode != MODE_PAR) printf("Temps d'execution sequentiel: %f\n", timeseq);
        printf("Temps d'execution parallele: %f\n", timepar);
        if (mode != MODE_PAR) printf("Accélération : %f\n", timeseq/timepar);
        if (mode != MODE_PAR) printf("Efficacité: %f\n", (timeseq/timeseq)/size);
    }
    else
    {
        double matrix[2][n][m];
        zeromatrix(n, m, 2, matrix);
        executepar(n, m, np, matrix);
    }
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}

void executeseq(int n, int m, int np, double matrix[2][n][m])
{
    int i, j, k;
    int current = 1;
    
    for (k = 1; k <= np; k++)
    {
        for (i = 1; i < n - 1; i++)
        {
            for (j = 1; j < m - 1; j++)
            {
                usleep(5);
                matrix[current][i][j] = (1.0 - 4*td / h*h) * matrix[1-current][i][j] +
                    (td/h*h) * (matrix[1-current][i - 1][j] +
                                matrix[1-current][i + 1][j] +
                                matrix[1-current][i][j - 1] +
                                matrix[1-current][i][j + 1]);
            }
        }
        current = 1 - current;
    }
}

void executepar(int n, int m, int np, double matrix[2][n][m])
{
    int i, j, k;
    int workers, tasks, extra, offset, left, right, rows;
    MPI_Status status;
    workers = size > n ? n : size - 1;
    
    if (id == MASTER)
    {
        direction = HORIZONTAL;
        tasks   = (direction == HORIZONTAL) ? n / workers : m / workers;
        extra   = (direction == HORIZONTAL) ? n % workers : m % workers;
        offset  = 0;
        
        //Send to workers
        for(i = 1; i <= workers; i++)
        {
            rows = (i <= extra) ? tasks + 1 : tasks;
            left = (i == 1) ? NO_NEIGH : i - 1;
            right = (i == workers) ? NO_NEIGH : i + 1;
            
            MPI_Send(&rows, 1, MPI_INT, i, BEGIN_TAG, MPI_COMM_WORLD);
            MPI_Send(&offset, 1, MPI_INT, i, BEGIN_TAG, MPI_COMM_WORLD);
            MPI_Send(&left, 1, MPI_INT, i, BEGIN_TAG, MPI_COMM_WORLD);
            MPI_Send(&right, 1, MPI_INT, i, BEGIN_TAG, MPI_COMM_WORLD);
            MPI_Send(&direction, 1, MPI_INT, i, BEGIN_TAG, MPI_COMM_WORLD);
            if (direction == HORIZONTAL)
            {
                MPI_Send(&matrix[0][offset][0], rows*m, MPI_DOUBLE, i, BEGIN_TAG, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(&matrix[0][0][offset], rows*n, MPI_DOUBLE, i, BEGIN_TAG, MPI_COMM_WORLD);
            }
            
            offset = offset + rows;
        }
        
        // Receive from workers
        for (i = 1; i <= workers; i++)
        {
            MPI_Recv(&offset, 1, MPI_INT, i, END_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, i, END_TAG, MPI_COMM_WORLD, &status);
            if (direction == HORIZONTAL)
            {
                MPI_Recv(&matrix[0][offset][0], rows*m, MPI_DOUBLE, i, END_TAG, MPI_COMM_WORLD, &status);
            }
            else
            {
                MPI_Recv(&matrix[0][0][offset], rows*n, MPI_DOUBLE, i, END_TAG, MPI_COMM_WORLD, &status);
            }
        }
    }
<<<<<<< HEAD

    printf("MPI_Barrier");
    //MPI_Barrier()
    err = MPI_Barrier(MPI_COMM_WORLD);

    //Demarrer le timer
    gettimeofday (&tp, NULL);
    timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

    //Effectuer le traitement parallele
    // process maitre qui gerent la reponse
    if(mon_id == master){
        printf("master\n");
        int proc_id = 1;
        MPI_Request requestList[(m-2)*(n-2)]; // -2 because of frontier
        MPI_Status statList[(m-2)*(n-2)]; // -2 because of frontier

        for(int k = 1; k <= np; k++){
            int sendCount = 0;
            for(int i = 1; i < m-1; i++){
                for(int j = 1; j < n-1; j++){
                    float data[5];
                    //set data
                    data[0] = matrix[i][j][(k+1)%2];   // cell to calculate
                    data[1] = matrix[i][j-1][(k+1)%2]; // left neighbor
                    data[2] = matrix[i-1][j][(k+1)%2]; // top neighbor
                    data[3] = matrix[i][j+1][(k+1)%2]; // right neigbor
                    data[4] = matrix[i+1][j][(k+1)%2]; // bottom neighbor

                    //send data to slave
                    MPI_Send(data,5,MPI_FLOAT,proc_id,SEND_TAG,MPI_COMM_WORLD);

                    //receive result from slave
                    //MPI_Irecv(&matrix[i][j][k%2],1,MPI_FLOAT,proc_id,MPI_ANY_TAG,MPI_COMM_WORLD,&requestList[sendCount]);
                    MPI_recv(&matrix[i][j][k%2],1,MPI_FLOAT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&statut);
                    sendCount++;
                    proc_id = (proc_id%(nbproc-1))+1;//get next process
                }
            }

            //Active wait for all data to be calculate to go to next delta time
            MPI_Waitall(sendCount,requestList,statList);
=======
    else if (id <= workers)
    {
        int left, right, rows, offset, begin, end;
        
        MPI_Recv(&rows, 1, MPI_INT, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&offset, 1, MPI_INT, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&left, 1, MPI_INT, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&right, 1, MPI_INT, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&direction, 1, MPI_INT, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
        if (direction == HORIZONTAL)
        {
            MPI_Recv(&matrix[0][offset][0], rows*m, MPI_DOUBLE, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
>>>>>>> 1cda73205ed0175b4b8832f26c92d7cb08a3bd33
        }
        else
        {
            MPI_Recv(&matrix[0][0][offset], rows*n, MPI_DOUBLE, MASTER, BEGIN_TAG, MPI_COMM_WORLD, &status);
        }
        
        
        begin = offset;
        end = offset + rows - 1;
        if (offset == 0)
        {
            begin = 1;
        }
        if ((direction == HORIZONTAL && offset + rows == n) || (direction == VERTICAL && offset + rows == m))
        {
            end--;
        }
    
        int jend = (direction == HORIZONTAL) ? m : n;
        int current = 0;
        
        for (k = 1; k <= np ; k++)
        {
            if (direction == HORIZONTAL)
            {
                // Left neighbor
                if (left != NO_NEIGH)
                {
                    MPI_Send(&matrix[current][offset][0], m, MPI_DOUBLE, left, TRANSMIT_LEFT_TAG, MPI_COMM_WORLD);
                    MPI_Recv(&matrix[current][offset - 1][0], m, MPI_DOUBLE, left, TRANSMIT_RIGHT_TAG, MPI_COMM_WORLD, &status);
                }
                
                // Right neighbor
                if (right != NO_NEIGH)
                {
                    MPI_Send(&matrix[current][offset + rows - 1][0], m, MPI_DOUBLE, right, TRANSMIT_RIGHT_TAG, MPI_COMM_WORLD);
                    MPI_Recv(&matrix[current][offset + rows][0], m, MPI_DOUBLE, right, TRANSMIT_LEFT_TAG, MPI_COMM_WORLD, &status);
                }
                
                for (i = begin; i <= end; i++)
                {
                    for (j = 1; j < jend - 1; j++)
                    {
                        usleep(5);
                        matrix[1-current][i][j] = (1.0 - 4*td / h*h) * matrix[current][i][j] +
                            (td/h*h) * (matrix[current][i - 1][j] +
                                        matrix[current][i + 1][j] +
                                        matrix[current][i][j - 1] +
                                        matrix[current][i][j + 1]);
                    }
                }
            }
            else
            {
                // Left neighbor
                if (left != NO_NEIGH)
                {
                    MPI_Send(&matrix[current][0][offset], n, MPI_DOUBLE, left, TRANSMIT_LEFT_TAG, MPI_COMM_WORLD);
                    MPI_Recv(&matrix[current][0][offset - 1], n, MPI_DOUBLE, left, TRANSMIT_RIGHT_TAG, MPI_COMM_WORLD, &status);
                }
                
                // Right neighbor
                if (right != NO_NEIGH)
                {
                    MPI_Send(&matrix[current][0][offset + rows - 1], n, MPI_DOUBLE, right, TRANSMIT_RIGHT_TAG, MPI_COMM_WORLD);
                    MPI_Recv(&matrix[current][0][offset + rows], n, MPI_DOUBLE, right, TRANSMIT_LEFT_TAG, MPI_COMM_WORLD, &status);
                }
                
                for (i = begin; i <= end; i++)
                {
                    for (j = 1; j < jend - 1; j++)
                    {
                        usleep(5);
                        matrix[1-current][j][i] = (1.0 - 4*td / h*h) * matrix[current][j][i] +
                        (td/h*h) * (matrix[current][j - 1][i] +
                                    matrix[current][j + 1][i] +
                                    matrix[current][j][i - 1] +
                                    matrix[current][j][i + 1]);
                    }
                }
            }
            current = 1 - current;
        }
        
        MPI_Send(&offset, 1, MPI_INT, MASTER, END_TAG, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, MASTER, END_TAG, MPI_COMM_WORLD);
        
        if (direction == HORIZONTAL)
        {
            MPI_Send(&matrix[current][offset][0], rows*m, MPI_DOUBLE, MASTER, END_TAG, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Send(&matrix[current][0][offset], rows*n, MPI_DOUBLE, MASTER, END_TAG, MPI_COMM_WORLD);
        }
    }
}

void initmatrix(int rows, int columns, double matrix[rows][columns])
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < columns; j++)
        {
            matrix[i][j] = i * j * (rows - i - 1) * (columns - j - 1);
        }
    }
}

void zeromatrix(int rows, int columns, int iterations, double matrix[iterations][rows][columns])
{
    int i, j, k;
    for (k = 0; k < iterations; k++)
    {
        for (i = 0; i < rows; i++)
        {
            for (j = 0; j < columns; j++)
            {
                matrix[k][i][j] = 0.0;
            }
        }
    }
    
}

void printmatrix(int rows, int columns, double matrix[rows][columns])
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < columns; j++)
        {
            printf("%5.2f\t", matrix[i][j]);
        }
        printf("\n");
    }
}

double getcurrenttime()
{
    struct timeval tp;
    gettimeofday (&tp, NULL);
    return (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
}
