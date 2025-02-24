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
        ptr_lst[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptr_lst[i]);
    }
}