#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mymalloc.h"

// Test 1: Repeat the process of allocating 1 byte and immediately freeing it 120 times
void stress_test1() {
    for (int i = 0; i < 120; i++) {
        void *ptr = malloc(1);
        free(ptr);
    }
}

// Test 2: 120개의 1바이트를 할당한 후에, 한번에 free()를 실행
void stress_test2() {
    void *ptr_lst[120];

    for (int i = 0; i < 120; i++) {
        printf("%d\n", i);
        ptr_lst[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptr_lst[i]);
    }
}

// Test 3: 
void stress_test3() {
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

// Test 4:
void stress_test4() {
    void* arr[120];
    for(int i = 0; i < 120; i++){
        int whatDo = rand();
        int arrIndex = 0;
        if(whatDo >= RAND_MAX / 2){ 
            while(arrIndex < 120){
                if(arr[arrIndex] == NULL){
                    arr[arrIndex] = malloc((rand() % 512)*8);
                    arrIndex = 0;
                    break;
                }
                arrIndex++;
            }
        } else {
            while(arrIndex < 120){
                if (arr[arrIndex] != NULL && *(int *)(arr[arrIndex] - 4)) {
                    free(arr[arrIndex]);
                    arrIndex = 0;
                    break;
                }
                arrIndex++;
            }
        }
    }
    for (int i = 0; i < 120; i++) {
        if (*(int *)(arr[i] - 4) == 1) {
            free(arr[i]);
        }
    }
}
// Test 5: 
void stress_test5() {
    void *arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = malloc(rand() % 17 + 2);
    }
    for (int i = 0; i < 100; i+=2) {
        if (arr[i] != NULL) {
            free(arr[i]);
            printf("%d\n", i);
        }    
    }
    for (int i = 1; i < 100; i+=2) {
        free(arr[i]);
    }
}

// Test 6
void leak_test() {
    void *ptr = malloc(1);
    free(ptr);
}

// Test 7: Edit version of test 3
void stress_test7() {
    void *arr[120] = {NULL};
    for(int i = 0; i < 120; i++){
        arr[i] = NULL;
    }
    int count = 0;
    while (count < 120) {
        int random_number = rand() % 120;
        int random_choice = rand() % 2;
        if (random_choice == 1 && arr[random_number] == NULL) {
            void *temp = malloc(1);
            if (temp != NULL) {
                arr[random_number] = temp;
                count++;
            } else {
                fprintf(stderr, "Error: malloc failed at index %d\n", random_number);
            }
        } 
        if (random_choice == 0 && arr[random_number] != NULL) {
            free(arr[random_number]);
            arr[random_number] = NULL;
            printf("free success\n");
        }
    }
    for (int i = 0; i < 120; i++) {
        if (arr[i] != NULL) {
            printf("freeing %d\n", i);  // index 44 badpointer, check it in arrange or not
            free(arr[i]);
            arr[i] = NULL;
        }
    }
}

// Test 8: Edit version of test 4



int main() {
    //stress_test2();
    stress_test3();
    //stress_test4();
    //stress_test5();
    //leak_test();
    //stress_test7();
    return 0;
}