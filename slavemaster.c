#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define TASKS  1000
#define TASK_SIZE 10000

#define DATA_TAG 0
#define DONE_TAG 2

int cmpfunc (const void* a, const void* b){
	return (*(int*)a - *(int*)b);
}


int main(int argc, char **argv)
{

    int my_rank;      // Identificador deste processo
    int proc_n;         // Numero de processos disparados pelo usuário na linha de comando (np)
    
    int** bag;     // Bag of work
    int* task;     //working task

    int next_task;
    int complete_tasks;

    int sopranaomenar = 0;

    int* processes; //table for slaveid -> current_task

    MPI_Status status;
    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o código paralelo esta abaixo
  
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD,  &proc_n);  // pega informação do numero de processos (quantidade total)
    double start_time;
 
    // Inicializa todos os nodos
    if (my_rank == 0)
    {
        start_time = MPI_Wtime();
        bag = malloc(sizeof(int*) * TASKS);
        bag[0] = malloc(TASKS * sizeof(int) * TASK_SIZE);    // sim, sou o primeiro, crio a mensagem sem receber
        
        int i;
        for(i = 1; i < TASKS; i++)
            bag[i] = bag[0] + i * TASK_SIZE;

        processes = malloc(sizeof(int) * proc_n);
        for(i = 1; i < proc_n; i++)
            processes[i] = 0;
        
	for(i = 0; i< TASKS * TASK_SIZE; i++)
		bag[0][i] = (TASKS * TASK_SIZE) - i;
	int j;
	// for(i = 0; i < TASKS; i ++){
	// 	for(j = 0; j < TASK_SIZE; j++){
	// 		printf("%d ", bag[i][j]);
	// 	}
	// 	printf("\n");
	// }

        next_task = 0;
        complete_tasks = 0;

    } else
        task = malloc(sizeof(int) * TASK_SIZE);


    // Data flow
    if (my_rank == 0)
    {
        //Primeira rajada
        int n;
        for(n=1; n<proc_n; n++)
        {
            processes[n] = next_task;
            MPI_Send(bag[next_task], TASK_SIZE, MPI_INT, n, DATA_TAG, MPI_COMM_WORLD);
            next_task++;
            printf(".");
        }

        //isso ta certo????
        while(next_task < TASKS)
        {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(bag[processes[status.MPI_SOURCE]], TASK_SIZE, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &status);
            complete_tasks++;
            processes[status.MPI_SOURCE] = next_task;
            MPI_Send(bag[next_task], TASK_SIZE, MPI_INT, status.MPI_SOURCE, DATA_TAG, MPI_COMM_WORLD);
            printf(".");
            next_task++;
        }

        //Manda todo mundo parar a zoeira
        //for(n=1; n<proc_n; n++)
        //    MPI_Send(&sopranaomenar, 0, MPI_INT, n, 2, MPI_COMM_WORLD);

        //Espera todo mundo parar de parar a zoeira. ordem?
        while(complete_tasks < TASKS)
        {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(bag[processes[status.MPI_SOURCE]], TASK_SIZE, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &status);
            complete_tasks++;
            //manda parar a zoeira.
            MPI_Send(&sopranaomenar, 1, MPI_INT, status.MPI_SOURCE, DONE_TAG, MPI_COMM_WORLD);
        }


	// int i,j;
	// for(i = 0; i < TASKS; i ++){
    //             for(j = 0; j < TASK_SIZE; j++){
    //                     printf("%d ", bag[i][j]);
    //             }
    //             printf("\n");
    //     }


        double end_time = MPI_Wtime();
        printf("time: %f\n", end_time - start_time);

    } else {

        while(1)
        {
            MPI_Recv(task, TASK_SIZE, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if(status.MPI_TAG == 0)
            {

		qsort(task, TASK_SIZE, sizeof(int), cmpfunc);

                //bubblesort
                // int k, l, m;
                // for(k=0;k<TASK_SIZE;k++){
                //     for(l=0;l<TASK_SIZE-1;l++){
                //         if(task[l] > task[l+1])
                //         {
                //             m = task[l];
                //             task[l] = task[l+1];
                //             task[l+1] = m;
                //         }
                //     }
                // }

                MPI_Send(task, TASK_SIZE, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD);
            }
            
            //recv para desbloquear master?
            if(status.MPI_TAG == 2)
                break;
                //MPI_Recv(&sopranaomenar, 1, MPI_INT, 0, DONE_TAG, MPI_COMM_WORLD, &status);
                
        }

    }


    MPI_Finalize();

}
