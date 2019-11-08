#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include "sys/time.h"

void mastercode (int nb_col,int nb_ligne, int np, double ***matrix);
void slavecode (double td, double h);
void init_matrix(int nb_col,int nb_ligne, double ***matrix);
void print_matrix0(int nb_col,int nb_ligne, double ***matrix);
void print_matrix1(int nb_col,int nb_ligne, double ***matrix);


int main (int argc, char *argv[])
{
  int nb_col, nb_ligne, np, rank, nprocs, i, j, x;
  double td, h, timeStart1, timeEnd1, Texec1, timeStart2, timeEnd2, Texec2;
  struct timeval tp1, tp2;

  //Initialisation des paramètres
  nb_col = atoi(argv[1]);
  nb_ligne = atoi(argv[2]);
  np = atoi(argv[3]);
  td = atof(argv[4]);
  h = atof(argv[5]);

  //Initialisation MPI
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  

  /************************SEQUENTIEL*****************************/
  //Initialisation de la matrice
  double matrix[nb_ligne][nb_col][2];
  init_matrix(nb_col, nb_ligne, (double ***) matrix);

  if(rank == 0){
    //Affichage de la matrice initiale
    printf("Matrice initiale séquentielle: \n");
    print_matrix0(nb_col, nb_ligne, (double ***) matrix);

    // Debut du chronometre
    gettimeofday (&tp1, NULL); 
    timeStart1 = (double) (tp1.tv_sec) + (double) (tp1.tv_usec) / 1e6;

    //Calcul séquentiel
    for(x = 1; x < np; x++){
      for(i = 1; i < nb_ligne-1; i++){
        for (j = 1; j < nb_col-1; j++){
          usleep(50);
          if(x%2 == 0)  matrix[i][j][0]=((1-(4*td)/(h*h))*matrix[i][j][1])+((td/(h*h))*(matrix[i-1][j][1]+matrix[i+1][j][1]+matrix[i][j-1][1]+matrix[i][j+1][1]));
          else          matrix[i][j][1]=((1-(4*td)/(h*h))*matrix[i][j][0])+((td/(h*h))*(matrix[i-1][j][0]+matrix[i+1][j][0]+matrix[i][j-1][0]+matrix[i][j+1][0]));
        }
      }
    }

    // Fin du chronometre
    gettimeofday (&tp1, NULL); 
    timeEnd1 = (double) (tp1.tv_sec) + (double) (tp1.tv_usec) / 1e6;
    Texec1 = timeEnd1 - timeStart1; //Temps d'execution en secondes 
    
    //Affichage matrice finale séquentielle
    printf("Matrice finale séquentielle: \n");
    if(np%2 == 0) print_matrix1(nb_col, nb_ligne, (double ***)matrix);
    else          print_matrix0(nb_col, nb_ligne, (double ***)matrix);
  }


  /************************PARALLELE*****************************/
  //Initialisation de la matrice
  init_matrix(nb_col, nb_ligne, (double ***)matrix);
  if(rank == 0){
    //Affichage de la matrice initiale
    printf("Matrice initiale parallèle: \n");
    print_matrix0(nb_col, nb_ligne, (double ***)matrix);
  }

  // Debut du chronometre
  gettimeofday (&tp2, NULL); 
  timeStart2 = (double) (tp2.tv_sec) + (double) (tp2.tv_usec) / 1e6;
  
  //Traitement parallel
  if (rank == 0)  mastercode (nb_col, nb_ligne, np, (double ***)matrix);
  else            slavecode (td, h);

  // Fin du chronometre
  gettimeofday (&tp2, NULL); 
  timeEnd2 = (double) (tp2.tv_sec) + (double) (tp2.tv_usec) / 1e6;
  Texec2 = timeEnd2 - timeStart2;

  //Affichage matrice finale séquentielle
  if(rank == 0){
    printf("Matrice finale parallele: \n");
    if(np%2 == 0) print_matrix1(nb_col, nb_ligne, (double ***)matrix);
    else          print_matrix0(nb_col, nb_ligne, (double ***)matrix);
  }

  if (rank == 0)  printf("temps dexécution séquentiel : %f, temps dexécution parallel : %f, accélération : %f\n", Texec1, Texec2, Texec1/Texec2);

  MPI_Finalize ();
  return 0;
}

void mastercode (int nb_col,int nb_ligne, int np, double ***matrix)
{
  int i=1, j=1, ii, jj, a=1, nprocs, rank=1, mat0=0, mat1=1, answers_to_receive, received_answers=0, send_answers=0, answers_to_send;
  double answer[3], task[7];
  MPI_Status status;
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  answers_to_receive = answers_to_send = nb_ligne*nb_col-2*nb_ligne-2*nb_col+4;

  while(a <= np){
    i=j=1;
    while(i<nb_ligne-1){
      while(j<nb_col-1){
        task[0] = matrix[i][j][mat0];
        task[1] = matrix[i+1][j][mat0];
        task[2] = matrix[i-1][j][mat0];
        task[3] = matrix[i][j+1][mat0];
        task[4] = matrix[i][j-1][mat0];
        task[5] = i;
        task[6] = j;
        MPI_Send (&task[0], 7, MPI_DOUBLE, rank%(nprocs-1)+1, 1, MPI_COMM_WORLD);
        send_answers++;
        fflush(stdout);
        if(send_answers == nprocs-1) break;
        j++;
        rank++;
      }
      if(send_answers == nprocs-1) break;
      i++;
      j = 1;
    }
    if(j >= nb_col-2){
      i++;
      j = 1;
    }
    else  j++;
    while (received_answers < answers_to_receive){
      MPI_Recv (&answer[0], 3, MPI_DOUBLE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
      fflush(stdout);
      ii = (int)(answer[1]);
      jj = (int)(answer[2]);
      matrix[ii][jj][mat1] = answer[0];
      received_answers++;

      if(send_answers < answers_to_send){
        task[0] = matrix[i][j][mat0];
        task[1] = matrix[i+1][j][mat0];
        task[2] = matrix[i-1][j][mat0];
        task[3] = matrix[i][j+1][mat0];
        task[4] = matrix[i][j-1][mat0];
        task[5] = i;
        task[6] = j;
        MPI_Send(&task[0], 7, MPI_DOUBLE, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
        send_answers++;
        if(j >= nb_col-2){
          i++;
          j = 1;
        }
        else  j++;
      }
    }
    fflush(stdout);
    received_answers = 0;
    send_answers = 0;
    a++;
    if(mat0 == 0) mat0 = 1;
    else          mat0 = 0;
    if(mat1 == 0) mat1 = 1;
    else          mat1 = 0;
  }
  //Envoi message de fin
  for (i = 1; i < nprocs; i++)  MPI_Send (&task[0], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
}

void slavecode (double td, double h)
{
  int rank, tag=1;
  double answer[3], task[7];
  MPI_Status status;

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  while(tag == 1){
    MPI_Recv(&task[0], 7, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    tag = status.MPI_TAG;
    usleep(5);
    answer[0] = ((1-(4*td)/(h*h))*task[0])+((td/(h*h))*(task[1]+task[2]+task[3]+task[4]));
    answer[1] = task[5];
    answer[2] = task[6];
    MPI_Send(&answer[0], 3, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
  }
}

void init_matrix(int nb_col,int nb_ligne, double ***matrix){
  int i, j;
  for (i = 0; i < nb_ligne; i++) {
    for (j = 0; j < nb_col; j++){
      matrix[i][j][0] = i*(nb_ligne-i-1)*j*(nb_col-j-1);
      matrix[i][j][1] = 0;
    }
  }
}

void print_matrix0(int nb_col,int nb_ligne, double ***matrix){
  int i, j;
  for (i = 0; i < nb_ligne; i++) {
    for (j = 0; j < nb_col; j++)  printf("%.2f    ", matrix[i][j][0]);
    printf("\n");
  }
}

void print_matrix1(int nb_col,int nb_ligne, double ***matrix){
  int i, j;
  for (i = 0; i < nb_ligne; i++) {
    for (j = 0; j < nb_col; j++)  printf("%.2f    ", matrix[i][j][1]);
    printf("\n");
  }
}