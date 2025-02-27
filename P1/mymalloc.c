#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mymalloc.h"
#define MEMLENGTH 4096  // heap memory size - 4096 bytes
int isIni = 0;  // check that the heap is initialized

static union {
    char bytes[MEMLENGTH];  // bytes = 4096
    double not_used;
} heap;

//This iterates through the LL, checking if any object is allocated.
void leak_detector() {
    // int num_bytes = 0;  no longer necessary as per class notes
    int num_objects = 0;
    int location = 0;
    while(location <= MEMLENGTH - 8){
        if(*(int *)(heap.bytes + location + 4) == 1){
            // num_bytes += *(int *)(heap.bytes + location); 
            num_objects++;
        }
        location += *(int *)(heap.bytes + location) + 8;
    }
    if(num_objects > 0){
        // printf("mymalloc: %d bytes leaked in %d objects", num_bytes, num_objects);
        // as mentioned in class, will not print byte leakage, since apparently it failed on the professor's code. If necessary for grading, please change.
        // num_bytes is already declared.
        printf("mymalloc: %d objects leaked\n", num_objects);
    }
}

// We allocate one object (the entire heap). at exit we run leak detector (As per writeup)
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
        // printf("%p\n", (int *)(heap.bytes));
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
                return (void *)(heap.bytes + location + 8); // return the next start address
            } else {    // if the block is larger
                *(int *)(heap.bytes + location) = size;     // set the size of the first split block
                *(int *)(heap.bytes + location + 4) = 1;    // set the first block as allocated

                *(int *)(heap.bytes + location + size + 8) = chunkSize - size - 8;  // set the size of the new block
                *(int *)(heap.bytes + location + size + 12) = 0; // set the new block as unallocated
                return (void *)(heap.bytes + location + 8); // return the next start address
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
    // This normalizes ptr input to the memory range of the heap. 
    // ie., we see if if the pointer we desire to be free can be freed by mymalloc, by checking the address difference -- in bytes(?)
    int diff = (char *)ptr - (char *)heap.bytes;    
    if (diff < 0 || diff >= MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s %d)\n", file, line);
        exit(2);
    } 
    // Condition 2
    // If the ptr is in the heap, we check if it really does point to a payload.
    // the only way to do this is to iterate through the list and see if our iterating ptr is ever "equal" to the desired ptr
    // there's no way to just directly see if it's not in some payload
    int location = 0;
    int found = 0;
    while (location <= MEMLENGTH - 8) {
        if ((int *)(heap.bytes + location + 8) == ptr) {
            found = 1;
            break;
        }
        location += *(int *)(heap.bytes + location) + 8;
    }
    // if we iterate through the whole list, we clearly have not found, therefore the ptr is bad
    if (!found) {
        fprintf(stderr, "free: Inappropriate pointer (%s %d)\n", file, line);
        exit(2);
    }
    // Condition 3
    // last check, are we freeing an allocated ptr?
    if (*(int *)(ptr - 4) == 0) {
        fprintf(stderr, "free: Inappropriate pointer (%s %d)\n", file, line);
        exit(2);
        // similiar with condition 2
    }
}

// This is run at the end of free (next function in the code)
// parse the LL, and if we find two consecutive chunks that are free, do a bitwise operation to change the size of the first free chunk
// Only checks next chunk if current check free (less work)
// leaves second free chunk's header as garbage data in the payload, there's no way to really "scrub/delete it"
void coalesce() {
    int location = 0;
    while (location <= MEMLENGTH - 8) {
        int currentSize = *(int *)(heap.bytes + location);
        int currentAllocation = *(int *)(heap.bytes + location + 4);
        if (currentAllocation == 0) {
            int nextChunk = location + currentSize + 8; // add the header size (8 bytes)
            if (nextChunk < MEMLENGTH) {    // check the coalescence is possible
                int nextAllocation = *(int *)(heap.bytes + nextChunk + 4);
                if (nextAllocation == 0) {  // check the next chunk's allocation status
                    int nextSize = *(int *)(heap.bytes + nextChunk);
                    *(int *)(heap.bytes + location) = currentSize + nextSize + 8;   // merge the sizes of two chunks, including the next chunk's header
                    currentSize = *(int *)(heap.bytes + location);
                    continue;
                }
            }
        }
        location += currentSize + 8;    // move to next chunk
    }
}

void myfree(void *ptr, char *file, int line) {
    badPointer(ptr, file, line); 
    *(int *)(ptr - 4) = 0;
    coalesce();
}
