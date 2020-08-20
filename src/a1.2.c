/*
    The sorting program to use for Operating Systems Assignment 1 2020
    written by Robert Sheehan

    Modified by: Yujia Wu
    UPI: ywu660

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <pthread.h>
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

unsigned long long int numOfThreads;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void print_data(struct block my_data) {
    for (int i = 0; i < my_data.size; ++i)
        printf("%d ", my_data.data[i]);
    printf("\n");
}

/* Split the shared array around the pivot, return pivot index. */
int split_on_pivot(struct block *my_data) {
    int right = my_data->size - 1;
    int left = 0;
    int pivot = my_data->data[right];
    while (left < right) {
        int value = my_data->data[right - 1];
        if (value > pivot) {
            my_data->data[right--] = value;
        } else {
            my_data->data[right - 1] = my_data->data[left];
            my_data->data[left++] = value;
        }
    }
    my_data->data[right] = pivot;
    return right;
}

/* Quick sort the data. */
void *quick_sort_thread(void *my_data) {
    struct block *cast = (struct block *)my_data;

    if (cast->size >= 2) {
        int pivot_pos = split_on_pivot(cast);

        struct block left_side, right_side;

        left_side.size = pivot_pos;
        left_side.data = cast->data;
        right_side.size = cast->size - pivot_pos - 1;
        right_side.data = cast->data + pivot_pos + 1;

      //   //Use a new threae to sort the left-side
      //   pthread_t thread_left;

      //   //If failed, then using the main thread like step 1
      //   if (pthread_create(&thread_left, NULL, quick_sort_thread, &left_side) != 0) {
      //           quick_sort(&left_side);
      //   }


      // //  Use a new thread to sort the right side
      //   pthread_t thread_right;

      //   //If failed, then using the main thread like step 1
      //   if (pthread_create(&thread_right, NULL, quick_sort_thread, &right_side) != 0) {
      //          quick_sort(&right_side);
      //   }

      //   //Wait for the left thread to finish
      //   pthread_join(thread_left, NULL);

      //   //Wait for the right thread to finish
      //   pthread_join(thread_right, NULL);
        quick_sort_thread(&left_side);
        quick_sort_thread(&right_side);
        
      
    }
}


/* Quick sort the data. */
void quick_sort(struct block my_data) {
    if (my_data.size < 2)
        return;
    int pivot_pos = split_on_pivot(&my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;

    // quick_sort(left_side);
    // quick_sort(right_side);

    //Use a new threae to sort the left-side
    pthread_t thread_left;

    //If failed, then using the main thread like step 1
    if (pthread_create(&thread_left, NULL, quick_sort_thread, &left_side) != 0) {
                quick_sort(left_side);
    }else {
        numOfThreads++;
    }

    //  Use a new thread to sort the right side
    pthread_t thread_right;

    //If failed, then using the main thread like step 1
    if (pthread_create(&thread_right, NULL, quick_sort_thread, &right_side) != 0) {
               quick_sort(right_side);
    }else {
        numOfThreads++;
    }

    //Wait for the left thread to finish
    pthread_join(thread_left, NULL);

    //Wait for the right thread to finish
    pthread_join(thread_right, NULL);
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

    //if the new thead is failed then using the main thread to sort like the step 1
    quick_sort(&start_block);

    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);

    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    printf("Number of threads created: %llu\n", numOfThreads);
    free(start_block.data);
    exit(EXIT_SUCCESS);
}