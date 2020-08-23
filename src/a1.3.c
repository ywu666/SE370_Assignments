/*
    The sorting program to use for Operating Systems Assignment 1 2020
    written by Robert Sheehan

    Modified by: Yujia Wu
    UPI: 

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/times.h>
#include <pthread.h>

#define SIZE    10

bool busy=true;
bool finishThread=false;
bool dataForThread = true;
struct block dataBelowPivot;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 


struct block {
    int size;
    int *data;
};

void print_data(struct block my_data) {
    for (int i = 0; i < my_data.size; ++i)
        printf("%d ", my_data.data[i]);
    printf("\n");
}

/* Split the shared array around the pivot, return pivot index. */
int split_on_pivot(struct block my_data) {
    int right = my_data.size - 1;
    int left = 0;
    int pivot = my_data.data[right];
    while (left < right) {
        int value = my_data.data[right - 1];
        if (value > pivot) {
            my_data.data[right--] = value;
        } else {
            my_data.data[right - 1] = my_data.data[left];
            my_data.data[left++] = value;
        }
    }
    my_data.data[right] = pivot;
    return right;
}

/* Quick sort the data. */
void quick_sort(struct block my_data);
void *quick_sort_by_thread(void *my_data) {
    //dataBelowPivot = (struct block*) my_data;
    printf("%s\n","dataBelowPivot" );
    print_data(dataBelowPivot);

    while(finishThread==false){
        if(dataForThread==true){
            busy=true;  
            dataForThread=false;         
            quick_sort(dataBelowPivot);
            busy=false;
        }else{
            pthread_mutex_lock(&lock);
            pthread_cond_wait(&cond,&lock);
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

/* Quick sort the data. */
void quick_sort( struct block my_data) {
    if (my_data.size < 2)
        return;
    int pivot_pos = split_on_pivot(my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;

    //quick_sort(right_side);

    if(busy==false){
        // there is data avaible for the second thread, always send the signal until it is received.
        dataForThread = true;
        dataBelowPivot.size = left_side.size;
        dataBelowPivot.data = left_side.data;
        printf("%s\n", "dataBelowPivot");
        print_data(dataBelowPivot);
        while(dataForThread == true){
            pthread_mutex_lock(&lock);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
        }
    }else{
        quick_sort(left_side);
        quick_sort(right_side);
    }

}



/* Check to see if the data is sorted. */
bool is_sorted(struct block my_data) {
    bool sorted = true;
    for (int i = 0; i < my_data.size - 1; i++) {
        if (my_data.data[i] > my_data.data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block my_data) {
    srand(1); // the same random data seed every time
    for (int i = 0; i < my_data.size; i++) {
        my_data.data[i] = rand() % 1000;
    }
}

int main(int argc, char *argv[]) {
	long size;

	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
    struct block start_block;
    start_block.size = size;
    start_block.data = (int *)calloc(size, sizeof(int));
    if (start_block.data == NULL) {
        printf("Problem allocating memory.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(start_block);

    if (start_block.size < 1001)
        print_data(start_block);

    
    struct tms start_times, finish_times;

    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);


    int pivot_pos = split_on_pivot(start_block);

    printf("pivot_pos: %d\n",pivot_pos);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = start_block.data;
    right_side.size = start_block.size - pivot_pos - 1;
    right_side.data = start_block.data + pivot_pos + 1;

    quick_sort(right_side);
    printf(is_sorted(right_side) ? "Right side is sorted\n" : "Right side is not sorted\n");

    pthread_t newThread;

    pthread_create(&newThread, NULL, quick_sort_by_thread, &left_side);

    finishThread = true;

    pthread_join(newThread, NULL);

    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    printf(is_sorted(left_side) ? "Left side is sorted\n" : "Left side is not sorted\n");
    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    free(start_block.data);
    exit(EXIT_SUCCESS);
}
