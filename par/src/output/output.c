#include <stdio.h>
#include <sys/time.h>

void printMatrix1(int dim, int * matrix) {
    for(int i = 0; i < dim; i += 8) {
		// Deroulage de boucle
    	printf("%12d ", matrix[i]);
    	printf("%12d ", matrix[i + 1]);
    	printf("%12d ", matrix[i + 2]);
    	printf("%12d ", matrix[i + 3]);
    	printf("%12d ", matrix[i + 4]);
    	printf("%12d ", matrix[i + 5]);
    	printf("%12d ", matrix[i + 6]);
    	printf("%12d ", matrix[i + 7]);

        printf("\n");
    }

    printf("\n");
}

void printMatrix2(int dim, int * matrix) {
	// Inversion des lignes avec les colonnes
    for(int i = 0; i < 8; ++i) {
		// Deroulage de boucle
    	printf("%12d ", matrix[i]);
    	printf("%12d ", matrix[i + 8]);
    	printf("%12d ", matrix[i + 16]);
    	printf("%12d ", matrix[i + 24]);
    	printf("%12d ", matrix[i + 32]);
    	printf("%12d ", matrix[i + 40]);
    	printf("%12d ", matrix[i + 48]);
    	printf("%12d ", matrix[i + 56]);

        printf("\n");
    }

    printf("\n");
}

// Affichage du temps de calcul
void printRuntime(struct timeval tvs, struct timeval tve) {
    long start = tvs.tv_sec * 1000000 + tvs.tv_usec;
    long end = tve.tv_sec * 1000000 + tve.tv_usec;
    long delta = end - start;
    printf("Runtime: %.6f seconds\n", delta / 1000000.0);
}
