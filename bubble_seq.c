#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TASK_SIZE  100000
#define MINIMUM_WORK  30
#define SELF_PERC    10 //task_size/self_perc = branch work

#define DATA_TAG  0
#define DONE_TAG  2

#define VERBOSE 1
#define VERBOSE_OUT 0

void printv(int* vector, int size){
    int g;
    for(g = 0; g < size; g++)
        printf("%d ", vector[g]);

}

void bubble_sort(int* vector, int size)
{
    int k, l, m;
    for(k=0;k<size;k++){
        for(l=0;l<size-1;l++){
            if(vector[l] > vector[l+1])
            {
                m = vector[l];
                vector[l] = vector[l+1];
                vector[l+1] = m;
            }
        }
    }
}
int main(int argc, char **argv)
{

    int my_rank;      // Identificador deste processo
    int proc_n;         // Numero de processos disparados pelo usuário na linha de comando (np)

    int* task;  //working task

    MPI_Status status;
    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD,  &proc_n);  // pega informação do numero de processos (quantidade total)
    double start_time;


    task = malloc(sizeof(int) * TASK_SIZE);

    int i;
    for(i = 0; i < TASK_SIZE; i++)
        task[i] = TASK_SIZE - i;

    start_time = MPI_Wtime();

    bubble_sort(task, TASK_SIZE);

    double end_time = MPI_Wtime();
    printf("Time!: %f", end_time - start_time);

    MPI_Finalize();

}
