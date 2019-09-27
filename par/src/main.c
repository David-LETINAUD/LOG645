#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>

#include "matrix/matrix.h"
#include "output/output.h"
#include "solver/solver.h"

#define DIM 64

int (* solve)(int iterations, int rank, int value) = solveFirst;

int main(int argc, char* argv[]) {
	// Variables temporelles
    struct timeval timestamp_s;
    struct timeval timestamp_e;

	// Récupération des parametres en argunents
    int problem = atoi(argv[1]);
    int initialValue = atoi(argv[2]);
    int iterations = atoi(argv[3]);

	// Selection de la 1ere ou la 2eme fonction selon le parametre problem
    void * solvers[2];
    solvers[0] = solveFirst;
    solvers[1] = solveSecond;

    solve = solvers[problem - 1];

	int number;
	int world_rank;
    int *rbuf = NULL;

	// Initialiser la bibliothèque MPI
    MPI_Init(NULL, NULL);
    
    // Sauvegarde du temps
	gettimeofday(&timestamp_s, NULL);

	// Obtenir le rang du processus d'appel dans le communicateur
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	// Si correspond au processus parents => allocation du buffer rbuf
	if (world_rank == 0) {
    	rbuf = malloc(sizeof(int) * DIM);
    }
	
	// Appel de la fonction de résolution de la matrice
	number = solve(iterations, world_rank, initialValue);
	
	// Rassemble les valeurs du groupe de 64 processus 
    MPI_Gather(&number, 1, MPI_INT, rbuf, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Si processus parent
    if (world_rank == 0) {
		// Affichage matrice finale
		if (problem == 1) {
			printMatrix1(DIM, rbuf);
		} else {
			printMatrix2(DIM, rbuf);
		}
		// Calcul et affichage du temps d'execution du processus parent
		gettimeofday(&timestamp_e, NULL);
		printRuntime(timestamp_s, timestamp_e);
    }
    
    // Nettoyer l'environnement d'exécution MPI  
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}
