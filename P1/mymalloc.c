#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mymalloc.h"
#define MEMLENGTH 4096  // heap memory size - 4096 bytes
int isIni = 0;  // check that the heap is initialized

static union {
    char bytes[MEMLENGTH];  // bytes = 4096
    double align;
} heap;

void leak_detector(){
    //int num_bytes = 0;
    int num_objects = 0;
    int location = 0;
    while(location <= MEMLENGTH - 8){
        if(*(int *)(heap.bytes + location + 4) == 1){
            //num_bytes += *(int *)(heap.bytes + location);
            num_objects++;
        }
        location += *(int *)(heap.bytes + location) + 8;
    }
    if(num_objects > 0){
        //printf("mymalloc: %d bytes leaked in %d objects", num_bytes, num_objects);
        printf("mymalloc: %d objects leaked", num_objects);
    }
}

void init_heap() {
    if (!isIni) {
        *(int *)(heap.bytes) = 4088;
        *(int *)(heap.bytes + 4) = 0;
        isIni = 1;
        atexit(leak_detector);
    }
}

void *mymalloc(size_t size, char *file, int line) {
    if (!isIni) {
        init_heap();    // initialize the heap if it has not been initialized
    }
    size = (size + 7) & ~7; // round up to the nearest multiple of 8 bytes
    int location = 0;   // start from the beginning of the heap

    while(location <= MEMLENGTH - 8) {
        size_t chunkSize = *(size_t *)(heap.bytes + location);    // the current chunk size
        int chunkAllocation = *(int *)(heap.bytes + location + 4);  // check the allocation status
        if (chunkSize >= size && chunkAllocation == 0) {    // if the current block is large enough and unallocated 
            if (chunkSize == size) {    //  if the block size exactly matches
                *(int *)(heap.bytes + location + 4) = 1;    // change the allocation status to 1
                return (void *)(heap.bytes + location + 8); // return the start address
            } else {    // if the block is larger
                *(int *)(heap.bytes + location) = size;     // set the size of the first split block
                *(int *)(heap.bytes + location + 4) = 1;    // set the first block as allocated

                *(int *)(heap.bytes + location + size + 8) = chunkSize - size;  // set the size of the new block
                *(int *)(heap.bytes + location + size + 12) = 0; // set the new block as unallocated
                return (void *)(heap.bytes + location + 8); // return the start address
            }
        } else { 
        location += chunkSize + 8;  // move to next block
        }
    }
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;   
}

void badPointer(void *ptr, char *file, int line) {
    // Condition 1
    int diff = (char *)ptr - (char *)heap.bytes;    // ptr가 heap.bytes 내부에 있는지 확인 (내부에 있으면 ptr가 heap.bytes보다 큼)
    if (diff < 0 || diff >= MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s.c%d)", file, line);
        exit(2);
    } 
    // Condition 2
    int location = 0;
    int found = 0;
    while (location <= MEMLENGTH - 8) {
        if ((int *)(heap.bytes + location + 8) == ptr) {
            found = 1;
            break;
        }
        location += *(int *)(heap.bytes + location) + 8;
    }
    if (!found) {
        fprintf(stderr, "free: Inappropriate pointer (%s.c%d)", file, line);
        exit(2);
    }
    // Condition 3
    if (*(int *)(ptr - 4) == 0) {
        fprintf(stderr, "free: Inappropriate pointer (%s.c%d)", file, line);
        exit(2);
        // similiar with condition 2
    }
}

void coalesce() {
    int location = 0;
    while (location <= MEMLENGTH - 8) {
        int currentSize = *(int *)(heap.bytes + location);
        int currentAllocation = *(int *)(heap.bytes + location + 4);
        if (currentAllocation == 0) {
            int nextChunk = location + currentSize + 8;
            if (nextChunk < MEMLENGTH) {
                int nextAllocation = *(int *)(heap.bytes + nextChunk + 4);
                if (nextAllocation == 0) {
                    int nextSize = *(int *)(heap.bytes + nextChunk);
                    *(int *)(heap.bytes + location) = currentSize + nextSize + 8;
                    currentSize = *(int *)(heap.bytes + location);
                    continue;
                }
            }
        }
        location += currentSize + 8;
    }
}

void myfree(void *ptr, char *file, int line) {
    badPointer(ptr, file, line); 
    *(int *)(ptr - 4) = 0;
    coalesce();
}
