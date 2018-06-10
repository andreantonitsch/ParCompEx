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


//Eu me recuso a usar {} quando preciso em um for ou if de um comando. --Agustini, 2018
//Challenge accepted
void merge(int*output, int* vector1, int size1, int* vector2, int size2, int* vector3, int size3)
{
int i, j, k, m;
i = j = k = m = 0;

    while(i <size1+size2+size3)
        if (j < size1 )
            if(k < size2)
                if(m<size3)
                    if(vector1[j] < vector2[k])
                            if(vector1[j] < vector3[m])
                                output[i++] = vector1[j++];
                            else
                                if(vector3[m] < vector2[k])
                                    output[i++] = vector3[m++];
                                else
                                    output[i++] = vector2[k++];
                    else
                        if(vector2[k] < vector3[m])
                            output[i++] = vector2[k++];
                        else
                            output[i++] = vector3[m++];
                else
                    if(vector1[j] < vector2[k])
                        output[i++] = vector1[j++];
                    else
                        output[i++] = vector2[k++];
            else
                if(m<size3)
                    if(vector1[j] < vector3[m])
                        output[i++] = vector1[j++];
                    else
                        output[i++] = vector3[m++];
                else
                    output[i++] = vector1[j++];
        else
             if(k<size2)
                if(m<size3)
                    if(vector2[k] < vector3[m])
                        output[i++] = vector2[k++];
                    else
                        output[i++] = vector3[m++];
                else
                    output[i++] = vector2[k++];
            else
                output[i++] = vector3[m++];
}

int main(int argc, char **argv)
{

    int my_rank;      // Identificador deste processo
    int proc_n;         // Numero de processos disparados pelo usuário na linha de comando (np)

    int* task;  //working task
    int* task1; //portion of task dividing nodes processs
    int* task2;
    int* task3;

    int current_task_size;
    int slice_work_size;
    int divide_size;

    MPI_Status status;
    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD,  &proc_n);  // pega informação do numero de processos (quantidade total)
    double start_time;

    //inicializa memorias
   int buffer_size = TASK_SIZE;

    task = malloc(sizeof(int) * buffer_size);

    //produces task
    if(my_rank == 0){
        int i;
        for(i = 0; i < TASK_SIZE; i++)
            task[i] = TASK_SIZE - i;

        //placeholder
        current_task_size = TASK_SIZE;
    	start_time = MPI_Wtime();

    }

    int father;
    //Receives task
    if(my_rank != 0){

	MPI_Recv(task, buffer_size, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &current_task_size);
	if(VERBOSE)
        	printf("%d Received Downward from: %d, Size: %d \n", my_rank, status.MPI_SOURCE, current_task_size);
        father = status.MPI_SOURCE;
    }

    //se existe trabalho a ser feito. work work work.
    if(my_rank < proc_n /2) //se node isn't certainly a leaf
    {
            //Divide
            if(current_task_size > MINIMUM_WORK){
                slice_work_size = current_task_size / SELF_PERC;
                divide_size = current_task_size - slice_work_size;


                task1 = malloc(sizeof(int) * current_task_size);
                task2 = malloc(sizeof(int) * current_task_size);
		        task3 = malloc(sizeof(int) * current_task_size);
                //Test
		if(VERBOSE){
                printf("%d Sending Downward to: %d, Size: %d \n", my_rank, (my_rank * 2) + 1, (divide_size/2));
                printf("%d Sending Downward to: %d, Size: %d \n", my_rank, (my_rank*2)+2, (divide_size - (divide_size / 2)) );
		}

                MPI_Send(task + slice_work_size, divide_size / 2, MPI_INT, (my_rank * 2)+1, DATA_TAG, MPI_COMM_WORLD);
                MPI_Send(task + slice_work_size + (divide_size / 2), divide_size - (divide_size / 2), MPI_INT, (my_rank * 2)+2, DATA_TAG, MPI_COMM_WORLD);

                memcpy(task1, task, slice_work_size * sizeof(int));

                bubble_sort(task1, slice_work_size);
		        //printv(task1, slice_work_size);

                int task2_leng;
                MPI_Recv(task2, buffer_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &task2_leng);
		if(VERBOSE)
                	printf("%d Received Msg3 from: %d, Size: %d \n", my_rank, status.MPI_SOURCE, task2_leng);

                int task3_leng;
                MPI_Recv(task3, buffer_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &task3_leng);
                if(VERBOSE)
			printf("%d Received Msg3 from: %d, Size: %d \n", my_rank, status.MPI_SOURCE, task3_leng);

                merge(task, task1, slice_work_size, task2, task2_leng, task3, task3_leng);

                if(my_rank != 0){
                    MPI_Send(task, current_task_size, MPI_INT, father, DONE_TAG, MPI_COMM_WORLD);
                }else{
                    //weeee dobby is freeeeee
                    double end_time = MPI_Wtime();
		    printf("Time!: %f", end_time - start_time);
		    if(VERBOSE_OUT)
			printv(task, current_task_size);
		}
            }

            //Conquer
            else{

                bubble_sort(task, current_task_size);
                MPI_Send(task, current_task_size, MPI_INT, father, DONE_TAG, MPI_COMM_WORLD);
            }

    }else{ //Node is a leaf
        bubble_sort(task, current_task_size);
	MPI_Send(task, current_task_size, MPI_INT, father, DONE_TAG, MPI_COMM_WORLD);
    }


    MPI_Finalize();
}
