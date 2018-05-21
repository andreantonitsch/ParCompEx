#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define TASKS = 7
#define TASK_SIZE = 100

int main(int argc, char **argv)
{

    int my_rank;      // Identificador deste processo
    int proc_n;         // Numero de processos disparados pelo usuário na linha de comando (np)
    
    int** bag;     // Bag of work
    int* task;     //working task

    int next_task;

    int* processes; //table for slaveid -> current_task

    MPI_Status status;
    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o código paralelo esta abaixo
  
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD,  &proc_n);  // pega informação do numero de processos (quantidade total)
    double start_time;
 
    // receber da esquerda
    if (my_rank == 0)
    {
        start_time = MPI_Wtime();
        bag = malloc(sizeof(int*) * TASKS);
        bag[0] = malloc(TASKS * sizeof(int) * TASK_SIZE);    // sim, sou o primeiro, crio a mensagem sem receber
        
        int i;
        for(i = 1; i < TASKS; i++)
            bag[i] = bag[0] + i * TASK_SIZE;

        processes = malloc(sizeof(int) * proc_n;
        for(i = 0; i < proc_n; i++)
            processes[i] = 0;
        
        next_task = 0;

    } else
        task = malloc(sizeof(int) * TASK_SIZE);

    int j;
    for (j = 0; j < rows; j++){
        if ( my_rank != 0 )    // sou o primeiro?
            MPI_Recv(message, proc_n, MPI_INT, my_rank-1, 1, MPI_COMM_WORLD, &status); // não sou o primeiro, recebo da esquerda

    // processo mensagem
       int k;
        for(k =0; k<rows; k++)
            message[my_rank] = my_rank * j; // incremento um na mensagem recebida

    // enviar para a direita
        if ( my_rank == proc_n-1 ){ // sou o último?
            int i = 0;
        }
        else
            MPI_Send(message, proc_n, MPI_INT, my_rank+1, 1, MPI_COMM_WORLD); // não sou o último, envio mensagem para a direita
    
    }

    if(my_rank == proc_n -1){
        int k =0;
        MPI_Send(&k, 1, MPI_INT, 0, 2, MPI_COMM_WORLD); 
    }

    if(my_rank == 0){
         int k;
         MPI_Recv(&k, 1, MPI_INT, proc_n-1, 2, MPI_COMM_WORLD, &status);

        double end_time = MPI_Wtime();
        printf("time: %f\n", end_time - start_time);
    }

    MPI_Finalize();

}
