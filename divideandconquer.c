#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define TASK_SIZE  100
#define MINIMUM_WORK  10
#define SELF_PERC    10 // task_size/self_perc = branch work

#define DATA_TAG  0
#define DONE_TAG  2
#define KILL_TAG  4


void bubble_sort(int* vector, int size)
{
    int k, l, m;
    for(k=0;k<size;k++){
        for(l=k;l<size-1;l++){
            if(vector[l] > vector[l+1])
            {
                m = vector[l];
                vector[l] = vector[l+1];
                vector[l+1] = m;
            }
        }
    }
}


void merge(int*output, int* vector1, int size1, int* vector2, int size2 )
{
    int i, j, k = 0;
    for(i = 0;i<size1+size2;i++){
        if(vector1[j] > vector2[k]){
            output[i] = vector2[k];
            k++;
        }else{
            output[i] = vector1[j];
            j++;
        }
    }
}

int main(int argc, char **argv)
{

    int my_rank;      // Identificador deste processo
    int proc_n;         // Numero de processos disparados pelo usuário na linha de comando (np)

    int* task;  //working task
    int* temp_task; //portion of task dividing nodes processs
    int* merge_task;

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
    int s;
    for(s = my_rank; s > 0; s /= 2)
    {
        buffer_size /= 2;
    }
    task = malloc(sizeof(int) * buffer_size);


    //produces task
    if(my_rank == 0){
        int i;
        for(i = 0; i < TASK_SIZE; i++)
            task[i] = TASK_SIZE - i;

        //placeholder
        current_task_size = TASK_SIZE;  
    }

    //Receives task
    if(my_rank != 0){

        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        //stop and stuff
        if(status.MPI_TAG == KILL_TAG){
            MPI_Recv(task, buffer_size, MPI_INT, status.MPI_SOURCE, KILL_TAG, MPI_COMM_WORLD, &status);
            current_task_size = -1;
        }
        
        if(status.MPI_TAG == DATA_TAG){
            MPI_Recv(task, buffer_size, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &status);
            current_task_size = task[0];
            task = task+1; //takes size int out of vector (loses some memory whoops)
        }
    }

    //se existe trabalho a ser feito. work work work.
    if(current_task_size > 0){

        if(my_rank < proc_n /2) //se node isn't certainly a leaf
        {
            //Divide
            if(current_task_size > MINIMUM_WORK){
                divide_size = current_task_size - (current_task_size / SELF_PERC);
                slice_work_size = current_task_size / SELF_PERC;

                temp_task =  malloc(sizeof(int) * (divide_size / 2)+1); 
                merge_task = malloc(sizeof(int) * current_task_size);

                //Test
                MPI_Send(task + slice_work_size, divide_size / 2, MPI_INT, (my_rank * 2)+1, DATA_TAG, MPI_COMM_WORLD);
                MPI_Send(task + slice_work_size + (divide_size / 2), divide_size - (divide_size / 2), MPI_INT, (my_rank * 2)+2, DATA_TAG, MPI_COMM_WORLD);

                bubble_sort(task, slice_work_size);

                //Fuse processed data
                int total_received_size = 0;

                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if(status.MPI_TAG == (my_rank * 2)+1)
                {
                    MPI_Recv(temp_task, divide_size / 2 , MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                    total_received_size = divide_size / 2;
                }else{
                    MPI_Recv(temp_task, divide_size - (divide_size / 2) , MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                    total_received_size = divide_size - (divide_size / 2);
                }

                merge(merge_task, task, slice_work_size, temp_task, total_received_size);
                total_received_size = total_received_size + slice_work_size;

                int last_message_size = 0;
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if(status.MPI_TAG == (my_rank * 2)+1)
                {
                    MPI_Recv(temp_task, divide_size / 2 , MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                    last_message_size = divide_size / 2;
                }else{
                    MPI_Recv(temp_task, divide_size - (divide_size / 2) , MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                    last_message_size = divide_size - (divide_size / 2);
                }

                merge(task, merge_task, total_received_size, temp_task, last_message_size);

                if(my_rank != 0){
                    MPI_Send(task + slice_work_size, divide_size / 2, MPI_INT, floor(my_rank / 2), DONE_TAG, MPI_COMM_WORLD);
                }else{
                    //weeee dobby is freeeeee
                    int g;
                    for(g = 0; g < TASK_SIZE; g++)
                        printf("%d ", task[g]);
                }
            }   
            
            //Conquer
            else{
                
                bubble_sort(task, current_task_size);
                MPI_Send(NULL,0 ,  MPI_INT, (my_rank * 2)+1, KILL_TAG, MPI_COMM_WORLD);
                MPI_Send(NULL, 0, MPI_INT, (my_rank * 2)+2, KILL_TAG, MPI_COMM_WORLD);
                MPI_Send(task, current_task_size, MPI_INT, floor(my_rank / 2), DONE_TAG, MPI_COMM_WORLD);

            }

        }else{ //Node is a leaf
            bubble_sort(task, current_task_size);
            MPI_Send(task, current_task_size, MPI_INT, floor(my_rank / 2), DONE_TAG, MPI_COMM_WORLD);
        }


    }

    MPI_Finalize();
}
