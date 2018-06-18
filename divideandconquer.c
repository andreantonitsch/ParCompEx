#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TASK_SIZE  1000000
#define MINIMUM_WORK  30
#define SELF_PERC    10 //task_size/self_perc = branch work

#define DATA_TAG  0
#define DONE_TAG  2

#define VERBOSE 0
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

void bs(int* vetor, int n)
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

void interleave (int result [] , int a [] , int b [], int c [], int msg_size, int a_size, int b_size, int c_size) {

  int i = 0;
  int j = 0;
  int k = 0;

  int position = 0;

  while (position < msg_size) {

      if ( i < a_size ) {

          if ( j < b_size ) {

              if ( k < c_size ) {

                  if ( a[i] < b[j] ) {

                      if ( a[i] < c[k] ) { result[position] = a[i]; i++; position++; }
                      else { result[position] = c[k]; k++; position++; }

                  } else {

                      if ( b[j] < c[k] ) { result[position] = b[j]; j++; position++; }
                      else { result[position] = c[k]; k++; position++; }

                  }

              } else { /* c ended */

                  if ( a[i] < b[j] ) { result[position] = a[i]; i++; position++; }
                  else { result[position] = b[j]; j++; position++; }

              }

          } else { /* b ended */

              if ( k < c_size ) {

                  if ( a[i] < c[k] ) { result[position] = a[i]; i++; position++; }
                  else { result[position] = c[k]; k++; position++; }

              } else { /* b and c ended */ result[position] = a[i]; i++; position++; }

          }

      } else if ( j < b_size ) { /* a ended */

          if ( k < c_size ) {

              if ( b[j] < c[k] ) { result[position] = b[j]; j++; position++; }
              else { result[position] = c[k]; k++; position++; }

          } else { /* a and c ended */ result[position] = b[j]; j++; position++; }

      } else if ( k < c_size ) { /* a and b ended */ result[position] = c[k]; k++; position++; }

  }

}
int* interleaveanny(int a[], int s, int i1, int i2, int i3)
{
	int* aux;
	int i1, i2, i3, i_aux;
	int diff = i3;

	aux = (int*) malloc(sizeof(int) * s);

	//i1 = 0;
	//i2 = diff;
	//i3 = (s - diff)/2 + diff;

	for (i_aux = 0; i_aux < s; i_aux++)
	{
		if (((a[i1] <= a[i2]) || (i2 == ((s-diff)/2 + diff))) 
		&& ((a[i1] <= a[i3]) || (i3 == s)) 
		&& (i1 < diff)) { aux[i_aux] = a[i1++]; }
		else
		{
			if (((a[i2] <= a[i1]) || (i1 == diff))
			&& ((a[i2] <= a[i3]) || (i3 == s))
			&& (i2 < ((s-diff)/2 + diff))) { aux[i_aux] = a[i2++]; } 
			else { aux[i_aux] = a[i3++]; }
		} 
	}
	return aux;
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

	    #if VERBOSE
        	printf("%d Received Downward from: %d, Size: %d \n", my_rank, status.MPI_SOURCE, current_task_size);
        #endif
        
        father = status.MPI_SOURCE;
    }

    task1 = malloc(sizeof(int) * current_task_size);
    //task2 = malloc(sizeof(int) * current_task_size);
    //task3 = malloc(sizeof(int) * current_task_size);


    //se existe trabalho a ser feito. work work work.
    if(my_rank < proc_n /2) //se node isn't certainly a leaf
    {
            //Divide
            if(current_task_size > MINIMUM_WORK){
                slice_work_size = current_task_size / SELF_PERC;
                divide_size = current_task_size - slice_work_size;

                task2 = task + slice_work_size;
                task3 = task + slice_work_size + (divide_size / 2);

                //Test

		        #if VERBOSE
                    printf("%d Sending Downward to: %d, Size: %d \n", my_rank, (my_rank * 2) + 1, (divide_size/2));
                    printf("%d Sending Downward to: %d, Size: %d \n", my_rank, (my_rank*2)+2, (divide_size - (divide_size / 2)) );
		        #endif

                MPI_Send(task2, divide_size / 2, MPI_INT, (my_rank * 2)+1, DATA_TAG, MPI_COMM_WORLD);
                MPI_Send(task3, divide_size - (divide_size / 2), MPI_INT, (my_rank * 2)+2, DATA_TAG, MPI_COMM_WORLD);

                //memcpy(task1, task, slice_work_size * sizeof(int));

                bs(task, slice_work_size);
		        //printv(task1, slice_work_size);

                int task2_leng;
                MPI_Recv(task2, buffer_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &task2_leng);

		        #if VERBOSE
                	printf("%d Received Msg3 from: %d, Size: %d \n", my_rank, status.MPI_SOURCE, task2_leng);
                #endif

                int task3_leng;
                MPI_Recv(task3, buffer_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &task3_leng);

                #if VERBOSE
			        printf("%d Received Msg3 from: %d, Size: %d \n", my_rank, status.MPI_SOURCE, task3_leng);
                #endif

                //interleave(task1, task, task2, task3, slice_work_size+task2_leng+task3_leng, slice_work_size, task2_leng, task3_leng);
                interleaveanny(task, current_task_size, slice_work_size, task2_leng, task3_leng);

                if(my_rank != 0){
                    MPI_Send(task1, current_task_size, MPI_INT, father, DONE_TAG, MPI_COMM_WORLD);
                }else{
                    //weeee dobby is freeeeee
                    double end_time = MPI_Wtime();
		            printf("Time!: %f", end_time - start_time);
                
                #if VERBOSE_OUT
                    printv(task1, current_task_size);
                #endif

		        }
            }

            //Conquer
            else{
                bs(task, current_task_size);
                MPI_Send(task, current_task_size, MPI_INT, father, DONE_TAG, MPI_COMM_WORLD);
            }

    }else{ //Node is a leaf
        bs(task, current_task_size);
	    MPI_Send(task, current_task_size, MPI_INT, father, DONE_TAG, MPI_COMM_WORLD);
    }


    MPI_Finalize();
}
