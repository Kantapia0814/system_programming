#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mymalloc.h"
#include <time.h>
#include <sys/time.h>

// Test 1: Repeat the process of allocating 1 byte and immediately freeing it 120 times
void stress_test1() {
    for (int i = 0; i < 120; i++) {
        void *ptr = malloc(1);
        free(ptr);
    }
}

// Test 2: Allocate 120 bytes (1 byte each) and free them all at once
void stress_test2() {
    void *ptr_lst[120];

    for (int i = 0; i < 120; i++) {
        //printf("%d\n", i);
        ptr_lst[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptr_lst[i]);
    }
}

// Test 3
void stress_test3() {
    void *arr[120];
    for(int i = 0; i < 120; i++){   // initialize all pointers to NULL
        arr[i] = NULL;
    }
    for(int i = 0; i < 120; i++){
        int whatDo = rand();
        int arrIndex = 0;
        if(whatDo >= RAND_MAX / 2){ // malloc or free with 50%
            while(arrIndex < 120){
                if(arr[arrIndex] == NULL){
                    arr[arrIndex] = malloc(1);
                    arrIndex = 0;
                    break;
                }
                arrIndex++;
            }
        } else {
            while(arrIndex < 120){
                if (arr[arrIndex] != NULL && *(int *)(arr[arrIndex] - 4) == 1) {
                    free(arr[arrIndex]);
                    arr[arrIndex] = NULL;
                    arrIndex = 0;
                    break;
                }
                arrIndex++;
            }
        }
    }
    for (int i = 0; i < 120; i++) {
        if (arr[i] != NULL && *(int *)(arr[i] - 4) == 1) {
            free(arr[i]);
        }
    }
}
// Test 4
void stress_test4() {
    void *arr[120];
    for(int i = 0; i < 120; i++){
        arr[i] = NULL;
    }
    for(int i = 0; i < 120; i++){
        int whatDo = rand();
        int arrIndex = 0;
        if(whatDo >= RAND_MAX / 2){ 
            while(arrIndex < 120){
                if(arr[arrIndex] == NULL){
                    srand(time(NULL));  // initialize with a new random seed
                    // this can't allocate more bytes than are in the heap, max is 511*8 = 4088
                    // it will fail a million times though (intentionally)
                    // not sure if conflicts with the not in (4.0) of the writeup
                    arr[arrIndex] = malloc((rand() % 512)*8); 
                    arrIndex = 0;
                    break;
                }
                arrIndex++;
            }
        } else {
            while(arrIndex < 120){
                if (arr[arrIndex] != NULL && *(int *)(arr[arrIndex] - 4) == 1) {
                    free(arr[arrIndex]);
                    arr[arrIndex] = NULL;
                    arrIndex = 0;
                    break;
                }
                arrIndex++;
            }
        }
    }
    for (int i = 0; i < 120; i++) {
        if (arr[i] != NULL && *(int *)(arr[i] - 4) == 1) {
            free(arr[i]);
        }
    }
}


// Test 5
void stress_test5() {
    void *arr[120];
    for (int i = 0; i < 120; i++) {
        arr[i] = malloc(rand() % 17 + 1);   // allocate memory between 1 bytes and 16 bytes
    }
    for (int i = 0; i < 120; i+=2) {
        if (arr[i] != NULL) {
            free(arr[i]);
            // printf("%d\n", i);
        }    
    }
    for (int i = 1; i < 120; i+=2) {
        if (arr[i] != NULL) {
            free(arr[i]);
            // printf("%d\n", i);
        }
    }
}

// we needed this bc code was bad, not anymore
void leak_test() {
    void *ptr = malloc(1);
    free(ptr);
}

// Test 6: Edit version of test 3
void stress_test6() {
    void *arr[120] = {NULL};
    for(int i = 0; i < 120; i++){
        arr[i] = NULL;
    }
    int count = 0;
    while (count < 120) {
        int random_number = rand() % 120;
        int random_choice = rand() % 2;
        if (random_choice == 1 && arr[random_number] == NULL) {
            void *temp = malloc(rand() % 120);
            if (temp != NULL) {
                arr[random_number] = temp;
                count++;    // count only when malloc succeeds
            } 
            // this is superflous, it always triggers when malloc fails.
            /* else {
                // If malloc fails to allocate, we print that the array entry could not be malloced
                fprintf(stderr, "Malloc failed at index %d\n", random_number);
            } */
        } 
        if (random_choice == 0 && arr[random_number] != NULL) {
            free(arr[random_number]);
            arr[random_number] = NULL;
            // printf("free success\n");
        }
    }
    for (int i = 0; i < 120; i++) {
        if (arr[i] != NULL) {
            // printf("freeing %d\n", i);  // index 44 badpointer, check it in arrange or not
            free(arr[i]);
            arr[i] = NULL;
        }
    }
}

int main() {
    struct timeval start, finish;
    double total_time = 0.0;

    for (int i = 0; i < 50; i++) {
        gettimeofday(&start, NULL);

        stress_test1();
        stress_test2();
        stress_test3();
        stress_test4();
        stress_test5();
        //leak_test();
        stress_test6();

        gettimeofday(&finish, NULL);

        double test_time = (finish.tv_sec - start.tv_sec) + (finish.tv_usec - start.tv_usec) / 1000000.0;
        total_time += test_time;
    }
    double average_time = total_time / 50;
    printf("Average time of test loop: %f sec\n", average_time);
    return 0;
}