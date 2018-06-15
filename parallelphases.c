#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TASK_SIZE 10000
#define SHARE_ZONE 1 / 5

#define DATA_TAG  0
#define TEST_TAG  1
#define DONE_TAG  2

#define VERBOSE 1
#define VERBOSE_OUT 0

void printv(int* vector, int size){
    int g;
    for(g = 0; g < size; g++)
        printf("%d ", vector[g]);

}

void bs(int n, int * vetor)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
        {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

int* vector_init(int* vector, int size, int start){
    int i = 0;
    while(i < size){
        vector[i] = start--;
    }
}

int checkready(int* vector, int size){
    
    int i;
    int result = 1;
    for(i=0; i<size; i++){
        result = result & vector[i];
    }

}

int main(int argc, char **argv)
{

    int my_rank;      // Identificador deste processo
    int proc_n;         // Numero de processos disparados pelo usuário na linha de comando (np)


    int* task;  //working task
    int* states;
    int share_size;

    MPI_Status status;
    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD,  &proc_n);  // pega informação do numero de processos (quantidade total)
    double start_time;

    int ready = 0;

    int slice_size = TASK_SIZE / proc_n;
    int share_size = floor((TASK_SIZE *  / proc_n) * SHARE_ZONE); 

    states = malloc(sizeof(int) * proc_n);

    int* task = malloc(sizeof(int) * size + 2*share_size);
    task += share_size;

    vector_init(task, TASK_SIZE / proc_n, my_rank * slice_size);

    int left_neighbour_biggust = -1;

    while(!ready){
        
        //local
        bs(slice_size, task);

        if(my_rank != proc_n-1)
            MPI_Send(task + slice_size -1, 1, MPI_INT, my_rank+1, TEST_TAG, MPI_COMM_WORLD);

        if(my_rank != 0)
            MPI_Recv(&left_neighbour_biggust, 1, MPI_INT, my_rank-1, TEST_TAG, MPI_COMM_WORLD);

        states[my_rank] = left_neighbour_biggust <= task[0];

        int i = 0;
        for(i=0; i< proc_n; i++){
            MPI_Bcast(states + i , 1, MPI_INT, i, MPI_COMM_WORLD)
        }
        
        //Check stop
        ready = checkready(states, proc_n);
        if(ready)
            break;
        
        //converge
        if(my_rank != 0)
            MPI_Send(task + slice_size - share_size, share_size, MPI_INT, my_rank-1, DATA_TAG, MPI_COMM_WORLD);

        if(my_rank != proc_n-1)
            MPI_Recv(task + slice_size, share_size, MPI_INT, my_rank-1, DATA_TAG, MPI_COMM_WORLD);
        
            bs(2*share_size, task + slice_size - share_size);

            MPI_Send(task + slice_size, share_size, MPI_INT, my_rank-1, DATA_TAG, MPI_COMM_WORLD);

        if(my_rank != 0)
            MPI_Recv(task + slice_size - share_size, share_size, MPI_INT, my_rank-1, DATA_TAG, MPI_COMM_WORLD);
    }   
    printf("rank %d: ", my_rank);
    printv(task, slice_size);

    MPI_Finalize();

}
