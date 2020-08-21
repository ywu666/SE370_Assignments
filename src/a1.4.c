/*
    The sorting program to use for Operating Systems Assignment 1 2020
    written by Robert Sheehan

    Modified by: put your name here
    UPI: put your login here

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

#define SIZE    10

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
void quick_sort(struct block my_data) {
    if (my_data.size < 2)
        return;
    int pivot_pos = split_on_pivot(my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;

    quick_sort(left_side);
    quick_sort(right_side);
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

    //Split the data by using the pivot point 
    int pivot_pos = split_on_pivot(start_block);
    struct block left_side, right_side;
    left_side.size = pivot_pos;
    left_side.data = start_block.data;
    right_side.size = start_block.size - pivot_pos - 1;
    right_side.data = start_block.data + pivot_pos + 1;

    struct tms start_times, finish_times;
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);

    int my_pipe[2];
    // Initialize the pipe
    if (pipe(my_pipe) == -1) {
        perror("Failed to create the pipe\n");
        exit(EXIT_FAILURE);
    }

    int c_pid = fork();
    if(c_pid  == -1) {
        perror("Failed to create the child\n");
        exit(EXIT_FAILURE);
    }

    if (c_pid != 0) { // the parent
         // Close pipe writer
        close(my_pipe[1]);


        //pass the data from the child process
        if(read(my_pipe[0], right_side.data, right_side.size * sizeof(int))){};


        quick_sort(left_side);
        printf(is_sorted(right_side) ? "right side is sorted\n" : "right side is not sorted\n");

        // Close the pipe reader so only reading allowed
        close(my_pipe[0]);

        times(&finish_times);
        printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);

        printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
        free(start_block.data);
        exit(EXIT_SUCCESS);
    } else { // the child

        //close the read one 
        close(my_pipe[0]);

        quick_sort(right_side);
        //printf(is_sorted(right_side) ? "Right side is sorted\n" : "right side is not sorted\n");
        if(write(my_pipe[1], right_side.data, right_side.size * sizeof(int))){}
        
        exit(EXIT_SUCCESS);
        close(my_pipe[1]);
    }
    // if (start_block.size < 1001)
    //     print_data(start_block);
}