// Name: Hoang Duong Nguyen
// id: a1876928
// Description: The pprogram will print out of place node
// Note: The array used for testing is right below this line. Modify as you see fit to test the program
int ARR[4] = {5,3,6,8}; // sample array for testing

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int getTargetP(int curr_rank , int max_rank , int direction) {  // A function that outputs the process num to send to when specified a direction. Input 0 for left and 1 for right
    int target_rank;

    if(direction) { // Going right, output rank +1
        target_rank = (curr_rank + 1) % max_rank;

    } else {
        target_rank = (curr_rank + max_rank - 1) % max_rank;
    }

    return target_rank;
}

int edgeCase(int curr_rank , int max_rank) {   // Resolve edgecase at end and beginning of the circle. Return 0 if not edge case, 1 if at end and -1 if at beginning
    if(curr_rank == (max_rank - 1) || curr_rank == 0) {
        return 1;

    } else if(curr_rank == 0) {
        return -1;
    }

    return 0;
}

void endProcess(int curr_rank) {
    printf("Process %d has now finished\n" , curr_rank);
    MPI_Finalize();
}

int main(void) {
    int p_rank;
    int comm_sz;
    int out_order_count = 0;
    int count = 0;

    int out_of_order_check = 1;
    int order_check = 0;

    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD , &p_rank);
    MPI_Comm_size(MPI_COMM_WORLD , &comm_sz);   // If run with 4 cores then size would be 4

    int node_data = ARR[p_rank];

    int target_R = getTargetP(p_rank , comm_sz , 1);
    int target_L = getTargetP(p_rank , comm_sz , 0);
    int data_L;
    int data_R;


    // Send to right
    MPI_Send(&node_data , 1 , MPI_INT , target_R , 0 , MPI_COMM_WORLD);
    //Recv from left - cause incoming data would be from left
    MPI_Recv(&data_L , 1 , MPI_INT , target_L , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);

    //Send to left
    MPI_Send(&node_data , 1 , MPI_INT , target_L , 1 , MPI_COMM_WORLD);
    //Recv from right
    MPI_Recv(&data_R , 1 , MPI_INT , target_R , 1 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);


    //Resolve edge case
    int edge = edgeCase(p_rank , comm_sz);
    if(node_data > data_L && edge == 1) {
        printf("Process %d is fine\n" , p_rank);
        MPI_Send(&order_check , 1 , MPI_INT , 0 , 2 , MPI_COMM_WORLD);

    } else if(node_data < data_R && edge == -1) {
        printf("Process %d is fine\n" , p_rank);

    } else if(edge) {
        printf("Process %d has at least one number out of order\n" , p_rank);
        if(p_rank == 0) {
            count++;
        } else if(edge) {
            MPI_Send(&out_of_order_check , 1 , MPI_INT , 0 , 2 , MPI_COMM_WORLD);
        }
    }

    // Check out of order
    if((node_data < data_L || node_data > data_R) && edge == 0) {
        printf("Process %d has at least one number out of order\n" , p_rank);
        MPI_Send(&out_of_order_check , 1 , MPI_INT , 0 , 2 , MPI_COMM_WORLD);

    } else if(edge == 0) {
        printf("Process %d is fine\n" , p_rank);
        MPI_Send(&order_check , 1 , MPI_INT , 0 , 2 , MPI_COMM_WORLD);
    }


    int count_buff;

    if(p_rank == 0) {
        for(int i=1 ; i<comm_sz ; i++) {
            MPI_Recv(&count_buff , 1 , MPI_INT , i , 2 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
            // printf("Received: %d\n" , count_buff);
            if(count_buff) {
                count++;
            }
        }

        printf("The number of processes holding an out-of-order number is %d\n" , count);
    }


    endProcess(p_rank);
    return 0;
}